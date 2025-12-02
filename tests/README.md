# automated test suite

this directory contains automated tests for the bread-final project features.

## test coverage

### test_tangent_bitangent
tests the mathematical correctness of tangent and bitangent calculations for all shape primitives.

**what it verifies:**
- all tangent, normal, and bitangent vectors are normalized (length = 1)
- all vectors are mutually orthogonal (dot product = 0)
- TBN forms a valid coordinate system (handedness check)

**shapes tested:**
- cube
- sphere
- cylinder
- cone

### test_texture_manager
tests the texture loading and management system.

**what it verifies:**
- valid textures can be loaded successfully
- invalid texture paths return 0 (error handling)
- texture caching works (same texture returns same id)
- texture binding to different units works correctly

## building the tests

the tests are integrated into the main project build system. from your normal build directory:

```bash
cd build
cmake ..
make
```

this will build:
- `BreadFinal` (the main application)
- `test_tangent_bitangent` (test executable)
- `test_texture_manager` (test executable)

## running the tests

from the build directory, run individual tests:

```bash
./test_tangent_bitangent
./test_texture_manager
```

or run all tests using ctest:

```bash
ctest --verbose
```

## interpreting results

each test outputs:
- `[PASS]` for successful tests with details
- `[FAIL]` for failed tests with error information
- summary showing total passed/failed count

the test executable returns:
- exit code 0 if all tests pass
- exit code 1 if any test fails

## test philosophy

these automated tests verify specific, measurable properties of the implementation:
- mathematical correctness (orthogonality, normalization)
- correct behavior (caching, error handling)
- opengl state consistency (texture binding)

no manual verification required - tests pass or fail deterministically.
