当PDF文件中的中文使用`WinAnsiEncoding`编码时，有些PDF阅读器可能会显示乱码，转为WORD文档时，也会是乱码，所以需要进行编码转换。

1. fix-s22pdf.js
在mupdf官方库中有一个[fix-s22pdf.js](https://github.com/ArtifexSoftware/mupdf/blob/master/docs/examples/fix-s22pdf.js)，这里做了一点改动。
这个脚本只能修改编码并将所有`宋体`，`黑体`，`楷体_GB2312`,`仿宋_GB2312`，`隶书`字体全部改为`宋体`，并不能做到按映射替换成多种字体。

2. replace_font.py
笔者使用python重新写了一个脚本：`replace_font.py`，实现了按映射替换成多种字体。

3. replace_builtin.c
`replace_builtin.c`是以C API的方式实现了[fix-s22pdf.js](https://github.com/ArtifexSoftware/mupdf/blob/master/docs/examples/fix-s22pdf.js)中的功能。

4. print_text.c
`print_text.c`以C API的方式实现在屏幕输出PDF中的文本内容
