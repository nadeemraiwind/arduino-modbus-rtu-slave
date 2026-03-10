# Arduino Library Manager Release Checklist

Use this checklist before creating a production tag.

## 1) Metadata and SemVer

- [ ] `library.properties` exists and is valid key=value format.
- [ ] `name` is stable and unique.
- [ ] `version` follows SemVer (`MAJOR.MINOR.PATCH`).
- [ ] `category` is set to `Communication` or another appropriate Arduino category.
- [ ] `architectures=*` if portable; otherwise list supported architectures explicitly.
- [ ] `url` points to the public repository.

## 2) Required Repository Files

- [ ] `src/` contains all public headers and implementation files.
- [ ] `examples/` contains the final production set only.
- [ ] `README.md` contains install, wiring, and quick-start guidance.
- [ ] `LICENSE` is present and matches repository license.
- [ ] `keywords.txt` includes public API symbols for IDE highlighting.
- [ ] `library.properties` is at repository root.

## 3) Production Layout Verification

- [ ] Root layout matches production intent:
  - `src/`
  - `examples/` (strict 01-08 lineup)
  - `docs/`
  - `library.properties`
  - `keywords.txt`
  - `README.md`
  - `LICENSE`
- [ ] Legacy/development artifacts are removed from release scope.

## 4) Build and Runtime Validation

- [ ] Compile examples for `arduino:avr:uno`.
- [ ] Compile examples for `arduino:avr:mega`.
- [ ] Regenerate Doxygen docs without warnings that indicate broken references.
- [ ] Confirm RS485 timing comments and field rules are present in user-facing docs.

## 5) Tag and Release Process

- [ ] Commit is clean and scoped to release-relevant files only.
- [ ] Create annotated tag matching `library.properties` version, e.g. `v1.0.0`.
- [ ] Push commit and tag to GitHub.
- [ ] Create GitHub release for the tag with changelog summary.

## 6) Arduino Library Manager Submission

- [ ] Repository is public.
- [ ] Tag is pushed and visible.
- [ ] Open/verify submission issue in `arduino/library-registry`.
- [ ] Ensure no folder nesting problem in release source structure.
- [ ] Verify examples open directly from Arduino IDE after install.

## 7) Post-Release Smoke Checks

- [ ] Install from released zip in Arduino IDE and compile Example 01.
- [ ] Confirm `Examples` menu shows expected 01-08 production lineup.
- [ ] Confirm `keywords.txt` highlighting appears in IDE.
