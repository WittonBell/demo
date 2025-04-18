package main

/*
#include "cgo.h"
#include <string.h>
*/
import "C"
import (
	"errors"
	"fmt"
	"reflect"
	"runtime"
	"syscall"
)

//export goTest
func goTest(str *C.char) *C.char {
	// C.GoString会使用Go语言分配内存，并复制到Go语言中，不需要手动释放内存，Go会自动GC
	// str是C语言中的内存，由C语言管理，Go不释放str内存
	fmt.Printf("Go使用的C字符串:%s\n", C.GoString(str))
	// C.CString会使用C语言分配内存，并复制到C语言中，需要手动使用C语言释放内存
	return C.CString("Go返回的C字符串")
}

//export goString
func goString() string {
	// 将Go的内存返回给C语言，在C语言中为只读内存
	return "Go语言返回的Go字符串"
}

func foo() {
	// C.CString会使用C语言分配内存，并复制到C语言中，需要手动使用C语言释放内存
	C.sayHello(C.CString("Go语言传递给C语言的C字符串"))
}

func test() {
	// 手动执行一下GC
	runtime.GC()
	// 测试之前在C语言中保存的Go语言中的字符串
	C.testGoString()
}

// CrtErrNo 在Windows下系统API的错误码与CRT的错误码冲突，Go默认使用的系统API的错误码
// 所以需要调用C函数来获取相应的错误提示字符串
func CrtErrNo(err error) string {
	var e syscall.Errno
	if errors.As(err, &e) {
		return C.GoString(C.strerror(C.int(err.(syscall.Errno))))
	}
	return err.Error()
}

func main() {
	fmt.Println("CGO开始")
	foo()
	test()
	sum, err := C.sum_positive(C.int32_t(1), C.int32_t(1))
	fmt.Println("和为：", sum)
	sum, err = C.sum_positive(C.int32_t(0), C.int32_t(1))
	if err != nil {
		// 输出错误类型
		fmt.Println(reflect.TypeOf(err))
		// 输出错误的具体含义
		fmt.Println(CrtErrNo(err))
		return
	}
}
