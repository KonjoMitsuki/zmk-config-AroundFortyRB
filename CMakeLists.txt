if(CONFIG_ARROUNDFORTY_STATUS_LED)
  # The widget has no public persistent-color API. The adapter uses its color
  # variable and message queue, so require the exact source we reviewed. Keep
  # this check and the revision in config/west.yml in sync when updating it.
  set(widget_source "${ZEPHYR_ZMK_RGBLED_WIDGET_MODULE_DIR}/src/widget.c")
  set_property(DIRECTORY APPEND PROPERTY CMAKE_CONFIGURE_DEPENDS "${widget_source}")
  file(SHA256 "${widget_source}" widget_hash)
  if(NOT widget_hash STREQUAL "bfd5f070e1b1cde4d274093272f3fc6c0918f0447d340ed764c4acd760d2282d")
    message(FATAL_ERROR "Review the AroundForty LED adapter before updating zmk-rgbled-widget")
  endif()
  if(CONFIG_RGBLED_WIDGET_SHOW_LAYER_CHANGE OR CONFIG_RGBLED_WIDGET_SHOW_LAYER_COLORS)
    message(FATAL_ERROR "Disable the widget's highest-layer indicators when using AroundForty status LEDs")
  endif()
  target_sources(app PRIVATE src/aroundforty_status_led.c)
endif()
