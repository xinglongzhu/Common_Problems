# readme

推荐使用  **nRF Connect for Desktop** 中的 **Toolchain Manager** 和 **VS Code** 开发

建议使用最新版本的 ncs 版本，旧版本可能会存在未知问题

**Attention**

为了使用自己的 net 核

在使用 distance_test 工程时，请在 nrf/samples/CMakeLists.txt 中的 add_child_image 中包装以下代码

```c
  if(!CONFIG_INCLUDE_USER_NET_CORE_IMAGE)

    add_child_image(
      NAME ${CHILD_IMAGE_NAME}
      SOURCE_DIR ${CHILD_IMAGE_PATH}
      DOMAIN "CPUNET"
      BOARD ${CONFIG_DOMAIN_CPUNET_BOARD}
      )
    endif()
    
  endif()
```

