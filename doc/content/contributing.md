# How to Contribute

Cardinal is a collaborative effort, and we encourage and welcome new contributions.

!alert tip
Before investing time in adding a new feature, it is recommended that you discuss
with the community and developer team about your contribution. An early discussion
can help ensure compatibility with Cardinal's standards for code modularity,
documentation, testing, and integration with larger systems in MOOSE, NekRS, and OpenMC.
To start a discussion, please visit the [Discussions](https://github.com/neams-th-coe/cardinal/discussions)
page.

# Work in a Fork

The first step is modifying Cardinal is to create your own
[fork](https://docs.github.com/en/get-started/quickstart/fork-a-repo) where you can
create branches and commit your set of changes.

### 1. Fork Cardinal

- Navigate to the [Cardinal repository](https://github.com/neams-th-coe/cardinal)
- Click the "Fork" icon in the upper right
- Clone your fork to your local machine; below, replace `username` with your GitHub username or
  the account where you forked the repository to

```
git clone https://github.com/username/cardinal.git
```

### 2. Add the `upstream` Remote:

Add the real Cardinal repository as a remote named "upstream":

```
cd cardinal
git remote add upstream https://github.com/neams-th-coe/cardinal.git
```

You should now have at least two remotes when you run `git remote -vv`:

```
origin https://github.com/username/cardinal.git (fetch)
origin https://github.com/username/cardinal.git (push)
upstream https://github.com/neams-th-coe/cardinal.git (fetch)
upstream https://github.com/neams-th-coe/cardinal.git (push)
```

### 3. Make Modifications

To make modifications to Cardinal, first create a branch where you will commit your
work.

```
git checkout -b branch_name upstream/devel
```

Then, add your contributions to the branch.

```
git add new_file.C
git commit -m "An informative message about the commit."
```

Before contributing your changes, you should rebase them on top of the latest
upstream "devel" branch in the real Cardinal repository:

```
git fetch upstream
git rebase upstream/devel
```

### 4. Add Documentation

Cardinal uses [MooseDocs](https://mooseframework.inl.gov/python/MooseDocs/index.html)
for documentation. Please be sure to document your changes and commit it to your working branch.

### 5. Push Modifications to GitHub

Push your branch to your fork:

```
git push origin branch_name
```

### 6. Create a Pull Request

Open a pull request in the main Cardinal repository to merge from your fork (the working
branch, `branch_name` that we have been working on) into the `devel` branch of the main
repository. For help with pull requests, please consult the [GitHub documentation](https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/proposing-changes-to-your-work-with-pull-requests/about-pull-requests).

When you open a pull request, please assign a Cardinal developer to review it.
They may have suggestions for changes; you can add changes again locally in your branch, with
new commits. Then, simply push to your fork with `git push origin branch_name`; this will
automatically reflect in the pull request you have opened in the main Cardinal repository.
