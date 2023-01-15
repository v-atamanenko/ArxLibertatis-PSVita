## Unfinished port of ArxLibertatis for PlayStation Vita.

Disclaimer: this compiles and boots but doesn't work.
Before had problems only with bad framerate, now doesn't load into menu due to
OOM (after syncing with upstream).

Needs vitagl built with `HAVE_HIGH_FFP_TEXUNITS=1` flag and Northfear's SDL2 fork.

Compile with
```bash
cmake -S. -Bbuild -DVITA=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build build -- -j$(nproc)
```

----------
Arx Libertatis is based on the publicly released Arx Fatalis source code. The source code is available under the GPLv3+ license with some additional terms - see the COPYING and LICENSE files for details.
