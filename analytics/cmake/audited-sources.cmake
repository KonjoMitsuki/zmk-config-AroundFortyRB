# ZMK v0.3.0, edf5c0814fd3ea202e43aad2d68fd32e882a518c.
# Paths are relative to zmk/app; content checks also work without .git metadata.
set(analytics_audited_sources
  "src/events/position_state_changed.c|123dfbc3b53fc86fd41df51d48d425936d2932603b31f30fd0786ff9da04b789"
  "src/event_manager.c|7910d1c1dd5ae69b0d59c42311ef74a121eda3192459911637fa370b60e8eccf"
  "include/zmk/event_manager.h|afe5a04e406aae30c65c5593ecbb6084758a6e0fde6ba569b6c2783c5d667c60"
  "include/zmk/events/position_state_changed.h|9e75d9f8b9405e3508723ca7a052f7d101faa03a5cdef0f2c86ae5bbde02e004"
  "src/combo.c|965af27e9f893181c7346372a36619c67e25469905cf5e7649dc6b50f791fa08"
  "src/physical_layouts.c|d39124f9f196658a2526be5caccd429a17d9f02ab98027a19073d0b6514c6f84"
  "src/split/central.c|ec39904d28420f5cfd72a6814d97d137e4ba03ca6d17be76cda91e9c14e61f28"
  "src/split/bluetooth/central.c|c82daf9d0b9423264375edb224123857f6c8f7d210eda065b6330add77ac651a"
)
foreach(entry IN LISTS analytics_audited_sources)
  string(REPLACE "|" ";" parts "${entry}")
  list(GET parts 0 path)
  list(GET parts 1 expected)
  file(SHA256 "${APPLICATION_SOURCE_DIR}/${path}" actual)
  if(NOT actual STREQUAL expected)
    message(FATAL_ERROR "Analytics PoC: unaudited ZMK source ${path}. Review observation paths and rerun regression tests before updating the digest.")
  endif()
endforeach()
