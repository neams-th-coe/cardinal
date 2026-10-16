# Shared helper, included by both CardinalMirrorSource.cmake and
# CardinalMirrorSubmodule.cmake, which calls cardinal_detect_mirror_
# divergence(...) right after their real --update rsync mirror run.
#
# Why this is needed: --update's whole job is to skip copying a file
# whenever the destination already looks newer than the source -- which
# is exactly what happens, silently and permanently, if someone edits a
# mirrored file (Cardinal's own source or an in-source contrib/ submodule
# like contrib/moose) directly in the build tree instead of the real
# source tree. The very next (and every subsequent) mirror run will see
# that edited file as "destination newer" and leave it alone forever:
# the edit is never reflected back into the real, git-tracked source
# tree, is invisible to `git status`/`git diff`, and is silently lost
# the moment the build directory is deleted. Confirmed live: a file
# edited directly in a mirrored build tree is never touched again by a
# normal `--update` rsync rerun, with no warning of any kind.
#
# Naive detection would just re-run the mirror as a checksum-based (-c),
# --update-free dry run (-n) over the whole tree every single time -- and
# an earlier version of this file did exactly that. Measured cost on the
# real checked-out Cardinal tree: ~0.86s for Cardinal's own ~240MB/5,900
# files, ~2.75s for contrib/moose's ~900MB/46,000 files -- roughly 4-7x
# the real mirror's own cost, paid on *every single build* (mirror_source
# and the submodule mirror both rerun on every build, not just the
# first), almost always to find nothing.
#
# Cheaper approach, used here: keep a stamp file (DEST_DIR/
# .cardinal_mirror_stamp) recording when the mirror last confirmed a
# clean state, and only ever checksum-compare the handful of files that
# have actually been touched in DEST_DIR since that stamp (found via a
# cheap `find -newer` stat walk, not a checksum pass) -- which is
# normally zero, and even during an active build is a tiny fraction of
# the whole tree. Correctness doesn't depend on this prefilter being
# exhaustive: the stat walk only decides what gets the *real* content
# check below, so excluding too little just costs a few more cheap
# per-file checks, never an incorrect result.
function(cardinal_detect_mirror_divergence SOURCE_DIR DEST_DIR)
  set(_stamp "${DEST_DIR}/.cardinal_mirror_stamp")

  if(NOT EXISTS "${_stamp}")
    # First time this destination has ever been mirrored -- nothing can
    # have diverged yet (the real sync above just populated it for the
    # first time). Establish the stamp -- strictly after that real sync
    # already finished writing every file, so none of those freshly
    # copied files show up as "newer than stamp" on the next invocation
    # -- and skip the check this time.
    file(TOUCH "${_stamp}")
    return()
  endif()

  # Build a `find` prune expression from the same exclude list the real
  # mirror just used (ARGN), so the candidate scan below stays out of
  # directories/patterns that are never actually mirrored -- most
  # importantly real build byproducts (CARDINAL_BYPRODUCT_EXCLUDES) and
  # other CMake/ExternalProject build trees (/build, /contrib, /install),
  # which get touched on essentially every single build and would
  # otherwise dominate the candidate list with files that were never
  # mirrored from SOURCE_DIR in the first place. This doesn't need to be
  # a byte-for-byte semantic match with rsync's own exclude matching --
  # it's purely a performance prefilter (see the file header); any
  # pattern this misses just means a few more candidates reach the
  # still-cheap, per-candidate content check below, never an incorrect
  # result.
  set(_prune_args -name .cardinal_mirror_stamp)
  foreach(_exclude ${ARGN})
    if(NOT _exclude MATCHES "^--exclude=(.+)$")
      continue()
    endif()
    set(_pattern "${CMAKE_MATCH_1}")
    list(APPEND _prune_args -o)
    if(_pattern MATCHES "^/")
      list(APPEND _prune_args -path "${DEST_DIR}${_pattern}")
    else()
      list(APPEND _prune_args -name "${_pattern}")
    endif()
  endforeach()

  execute_process(
    COMMAND find "${DEST_DIR}" "(" ${_prune_args} ")" -prune -o -type f -newer "${_stamp}" -print
    OUTPUT_VARIABLE _candidates_raw
    RESULT_VARIABLE _rc)
  if(_rc)
    message(WARNING "Cardinal: could not scan '${DEST_DIR}' for recently-touched files (find exited ${_rc}); skipping this build's divergence check.")
    return()
  endif()

  string(STRIP "${_candidates_raw}" _candidates_raw)
  if(_candidates_raw STREQUAL "")
    # Nothing in the mirrored scope has been touched since the last
    # mirror -- definitely nothing to flag. Advance the stamp and skip
    # the (otherwise unnecessary) content check entirely. This is the
    # common case on every incremental build.
    file(TOUCH "${_stamp}")
    return()
  endif()

  string(REPLACE "\n" ";" _candidates "${_candidates_raw}")
  set(_relpaths)
  foreach(_abspath ${_candidates})
    string(REPLACE "${DEST_DIR}/" "" _relpath "${_abspath}")
    list(APPEND _relpaths "${_relpath}")
  endforeach()

  # Targeted checksum check, restricted via --files-from to just the
  # handful of recently-touched candidates above -- not a full-tree
  # pass. A legitimate file the real sync above just copied in (because
  # SOURCE_DIR changed) will show up as a candidate here too (its mtime
  # is also newer than the old stamp), but will correctly show no
  # divergence below, since its content now matches SOURCE_DIR exactly.
  set(_filelist "${DEST_DIR}/.cardinal_mirror_candidates")
  string(REPLACE ";" "\n" _filelist_content "${_relpaths}")
  file(WRITE "${_filelist}" "${_filelist_content}\n")

  execute_process(
    COMMAND rsync -rlpgon -i -c --files-from=${_filelist} ${SOURCE_DIR}/ ${DEST_DIR}/
    OUTPUT_VARIABLE _itemize_output
    RESULT_VARIABLE _rc)
  file(REMOVE "${_filelist}")
  if(_rc)
    message(WARNING "Cardinal: could not check '${DEST_DIR}' for stray build-tree edits (rsync exited ${_rc}); skipping this check.")
    return()
  endif()

  # rsync's --itemize-changes (-i) output marks a checksum mismatch on an
  # already-existing destination file with a literal 'c' as the 3rd
  # character of its fixed 11-character flag field (`>fcsT...... path`);
  # a brand-new file not yet mirrored at all gets '+' there instead
  # (`>f+++++++++ path`) -- not a divergence, just something the real
  # sync above already picked up normally. Only the 'c' form is treated
  # as a divergence.
  string(REPLACE "\n" ";" _lines "${_itemize_output}")
  set(_diverged)
  foreach(_line ${_lines})
    if(_line MATCHES "^>f[c]")
      string(SUBSTRING "${_line}" 12 -1 _path)
      list(APPEND _diverged "${_path}")
    endif()
  endforeach()

  if(_diverged)
    # Deliberately do NOT advance the stamp here: leaving it where it is
    # means this same file (plus anything else touched since) keeps
    # showing up as a candidate on every future build until the
    # divergence is actually resolved, so the warning below keeps
    # reappearing rather than firing once and then going silent forever
    # if it's missed in a long build log. The ongoing cost of that stays
    # low regardless -- it's still just this small candidate set, not a
    # full-tree pass.
    list(JOIN _diverged "\n    " _diverged_pretty)
    message(WARNING
      "Cardinal: the following file(s) under '${DEST_DIR}' have been "
      "edited directly in the build tree and no longer match "
      "'${SOURCE_DIR}':\n"
      "    ${_diverged_pretty}\n"
      "These edits will NOT be picked up by git, will be silently kept "
      "forever (every future mirror run treats the build-tree copy as "
      "newer and never overwrites it), and will be lost the moment this "
      "build directory is deleted. Move the real edit into "
      "'${SOURCE_DIR}' instead. This warning will keep appearing on "
      "every build until fixed.")
  else()
    file(TOUCH "${_stamp}")
  endif()
endfunction()
