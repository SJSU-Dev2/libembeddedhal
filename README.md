<div align="center">
<img height="250" src="https://raw.githubusercontent.com/libhal/.github/main/profile/logo.png">
<br />
<br />

```
🚚 portable 🦾 flexible 📦 accessible 🍰 easy to use
```
<br />
</div>

<div align="center">

[![docs](https://github.com/libhal/libhal/actions/workflows/docs.yml/badge.svg?branch=main)](https://github.com/libhal/libhal/actions/workflows/docs.yml)
[![lint](https://github.com/libhal/libhal/actions/workflows/lint.yml/badge.svg?branch=main)](https://github.com/libhal/libhal/actions/workflows/lint.yml)
[![tests](https://github.com/libhal/libhal/actions/workflows/tests.yml/badge.svg?branch=main)](https://github.com/libhal/libhal/actions/workflows/tests.yml)
[![coverage](https://libhal.github.io/libhal/coverage/coverage.svg)](https://libhal.github.io/libhal/coverage/)
[![GitHub stars](https://img.shields.io/github/stars/libhal/libhal.svg)](https://github.com/libhal/libhal/stargazers)
[![GitHub forks](https://img.shields.io/github/forks/libhal/libhal.svg)](https://github.com/libhal/libhal/network)
[![GitHub issues](https://img.shields.io/github/issues/libhal/libhal.svg)](https://github.com/libhal/libhal/issues)
[![codecov](https://codecov.io/gh/libhal/libhal/branch/master/graph/badge.svg?token=E7aAr2o1CT)](https://codecov.io/gh/libhal/libhal)
[![Codacy Badge](https://app.codacy.com/project/badge/Grade/b084e6d5962d49a9afcb275d62cd6586)](https://www.codacy.com/gh/libhal/libhal/dashboard?utm_source=github.com&amp;utm_medium=referral&amp;utm_content=libhal/libhal&amp;utm_campaign=Badge_Grade)

</div>

# [:books: Interface APIs](https://libhal.github.io/libhal/api)

# :inbox_tray: Install

## `conanfile.txt`
Add the following line to your `conanfile.txt` file under the `[requires]`
section:

```
libhal/[x]
```

Replace `[x]` with the appropriate version number, otherwise `[x]` will pick the
latest available version of the library.

## `conanfile.py`

Add the following to your `def requirements(self):` function.

```python
self.requires("libhal/[x]")
```

Replace `[x]` with the appropriate version number, otherwise `[x]` will pick the
latest available version of the library.

# :white_check_mark: Supported Interfaces

# :busts_in_silhouette: Contributing

## :raised_hand: Self Assigning to an Issue

If you find an issue you'd like to work on, simply type and submit a comment
with the phrase `.take` in it to get assigned by our github actions.

## :pencil2: Pull Request Guidelines

1. Code must finish continuous integration steps before it will be reviewed.
2. Commit messages should follow these guidelines here
   https://cbea.ms/git-commit/.
3. Pull requests should contain a single commit
4. Pull requests should be small and implement a single feature where possible.
   If it can be broken up into separate parts it most likely should be.
5. Each PR should have an associated issue with it. Exceptions are made for very
   small PRs such as fixing typos, fixing up documentation or removing
   unnecessary headers.
