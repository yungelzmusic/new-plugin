# The Royal Strip

Developer: Elz Da Prynce

Channel strip: Gate > Plosive Control > Pitch Correct > Pre-amp > EQ > Dynamic EQ > De-esser > Compressor >
Multiband > Saturation > Glue > Air > Delay / Reverb (ducked) > Limiter.

## Build on your own machine
    cmake -B build -DCMAKE_BUILD_TYPE=Release
    cmake --build build --config Release
VST3 (and AU on Mac) are copied to your plugin folders automatically.

## Build in the cloud (no compiler needed)
1. Create a private GitHub repo and upload this folder.
2. Actions tab > "Build Royal Strip" > Run workflow.
3. Download the artifact for Windows or macOS when it finishes.

## Pro Tools (AAX)
1. Get the AAX SDK from your Avid developer account.
2. Build with  -DAAX_SDK_PATH=/path/to/aax-sdk  (or add the AAX_SDK_URL secret for the cloud build).
3. Retail Pro Tools only loads AAX plugins signed with PACE/iLok (Avid's wraptool). Apply for
   a signing account with Avid. Unsigned builds load in Pro Tools Developer only.
4. Put the signed .aaxplugin in:
   Windows: C:\Program Files\Common Files\Avid\Audio\Plug-Ins\
   macOS:   /Library/Application Support/Avid/Audio/Plug-Ins/
   (Sign with wraptool using the account and GUID Avid gives you; see Avid's wraptool docs for exact flags.)

## Features in this source update
- Global user preset save/load (`.rstrip`) and per-module starter preset menus.
- Mono and stereo main-bus layouts.
- Plosive-control module with adjustable detector frequency, threshold and amount.
- Pitch key and scale are dropdowns to make Chromatic / Major / Minor selection reliable.
- EQ / dynamic EQ response display (visual guide, not a measurement-grade analyzer).
- Resizable editor with compact/expanded sizing controls.

## Notes
- Reported latency is about 17 ms (pitch stage + limiter), always on, so Pro Tools can compensate.
- Pitch correct is a basic delay-line shifter, fine for small corrections, not Auto-Tune grade.
- Limiter is sample-peak, not true-peak. Saturation/pre-amp are not oversampled.
- Change COMPANY_NAME, Yrst, Rstr and AAX_IDENTIFIER in CMakeLists.txt before releasing.
