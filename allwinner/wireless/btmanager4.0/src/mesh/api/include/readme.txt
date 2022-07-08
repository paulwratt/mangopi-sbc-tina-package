框架设计笔记:

AW_MESH_ERROR_NOT_INIT_DONE:
    某些API 被调用时,发现dbus或者mesh app不存在,说明mesh stack 未初始化或未初始化完成,提示用户进行初始化或提示用户初始化失败.
AW_MESH_ERROR_INVALID_ARGS
    某些API的用户参数输入错误并且被检查出来是,返回改失败给用户.