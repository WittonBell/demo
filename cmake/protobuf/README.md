CMake自动按目录结构编译Protobuf代码

在MinGW下使用GCC编译器编译，Clang链接有问题：

```
[build] G:/msys64/mingw64/bin/ld: CMakeFiles/pb.dir/src/pb/t.pb.cc.obj: in function `absl::lts_20240116::log_internal::LogMessage::operator<<(unsigned long long)':
[build] G:/msys64/mingw64/include/absl/log/internal/log_message.h:135:(.text$_ZN4absl12lts_2024011612log_internal10LogMessagelsEy[_ZN4absl12lts_2024011612log_internal10LogMessagelsEy]+0x19): undefined reference to `_ZN4absl12lts_2024011612log_internal10LogMessagelsIyTnNSt9enable_ifIXntsr4absl16HasAbslStringifyIT_EE5valueEiE4typeELi0EEERS2_RKS5_'
[build] clang++: error: linker command failed with exit code 1 (use -v to see invocation)
[build] ninja: build stopped: subcommand failed.
```
