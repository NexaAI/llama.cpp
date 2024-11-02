file(REMOVE_RECURSE
  "libnexa-omni-audio-lib.pdb"
  "libnexa-omni-audio-lib.so"
)

# Per-language clean rules from dependency scanning.
foreach(lang CXX)
  include(CMakeFiles/nexa-omni-audio-lib_shared.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
