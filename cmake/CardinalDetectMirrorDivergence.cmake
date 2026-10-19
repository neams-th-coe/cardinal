# Shared helper, included by both CardinalMirrorSource.cmake and
# CardinalMirrorSubmodule.cmake, which calls cardinal_detect_mirror_
# divergence(...) right after their real --update rsync mirror run, to
# warn about a file edited directly in the build tree instead of the
# real source tree (which --update would otherwise leave diverged
# forever, silently). See doc/content/cmake_details.md ("Detecting edits
# made directly in the build tree") for the full design rationale,
# including why this uses a stamp file plus a targeted check rather than
# a full-tree checksum comparison on every build.
function(cardinal_detect_mirror_divergence SOURCE_DIR DEST_DIR)
  set(_stamp "${DEST_DIR}/.cardinal_mirror_stamp")

  if(NOT EXISTS "${_stamp}")
    # First time this destination has ever been mirrored -- nothing can
    # have diverged yet. Establish the stamp (after the real sync above
    # already finished) and skip the check this time.
    file(TOUCH "${_stamp}")
    return()
  endif()

  # Candidate scan: files touched in DEST_DIR since the last clean
  # check, pruned using the same exclude list the real mirror used (see
  # cmake_details.md -- this is a performance prefilter only, not a
  # correctness requirement).
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
    file(TOUCH "${_stamp}")
    return()
  endif()

  # Keep only candidates that exist at the same relative path under
  # SOURCE_DIR -- anything else is build-tree-only bookkeeping with no
  # source counterpart (CMakeFiles/, Makefile.cardinal, ...), which
  # would otherwise reach --files-from below as a path it can't resolve
  # and abort the whole batch rather than just that one entry.
  string(REPLACE "\n" ";" _candidates "${_candidates_raw}")
  set(_relpaths)
  foreach(_abspath ${_candidates})
    string(REPLACE "${DEST_DIR}/" "" _relpath "${_abspath}")
    if(EXISTS "${SOURCE_DIR}/${_relpath}")
      list(APPEND _relpaths "${_relpath}")
    endif()
  endforeach()

  if(NOT _relpaths)
    file(TOUCH "${_stamp}")
    return()
  endif()

  # Targeted checksum check, restricted via --files-from to just the
  # candidates above. A file the real sync just legitimately copied in
  # will also appear here (its mtime is newer than the old stamp too),
  # but will correctly show no divergence, since its content already
  # matches SOURCE_DIR.
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

  # rsync's --itemize-changes (-i) marks a checksum mismatch on an
  # already-existing file with 'c' as the 3rd character of its 11-char
  # flag field (`>fcsT...... path`); a brand-new, not-yet-mirrored file
  # gets '+' there instead (`>f+++++++++ path`) -- not a divergence.
  string(REPLACE "\n" ";" _lines "${_itemize_output}")
  set(_diverged)
  foreach(_line ${_lines})
    if(_line MATCHES "^>f[c]")
      string(SUBSTRING "${_line}" 12 -1 _path)
      list(APPEND _diverged "${_path}")
    endif()
  endforeach()

  if(_diverged)
    # Don't advance the stamp: leaving it in place means this file keeps
    # reappearing as a candidate (and the warning keeps reappearing)
    # until it's actually fixed, rather than firing once and going
    # silent if missed in a long build log.
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
