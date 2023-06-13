if (CONFIG_INCLUDE_USER_NET_CORE_IMAGE)
  add_child_image(
    NAME cpunet
    SOURCE_DIR ${CMAKE_CURRENT_LIST_DIR}/../cpunet
    DOMAIN cpunet
    BOARD ${CONFIG_DOMAIN_CPUNET_BOARD}
    )
endif()
