@echo off
rd cpp /q /s
md cpp

for %%f in (*.proto) do (
  @echo Compile %%f
  protoc --cpp_out=cpp %%f
)

pause