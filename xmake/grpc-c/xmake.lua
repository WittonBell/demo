add_requires("grpc","protobuf-c")

rule("grpc-c")
	set_extensions(".proto")
	on_prepare_file(function(target, file, opt)
		import("core.project.depend")
		import("utils.progress")
		import("lib.detect.find_file")
		target:add("packages", "grpc", "protobuf-c")
		local output_dir = target:autogendir()
		os.mkdir(output_dir)
		local basename = path.basename(file)
		local pbSourceFile = path.join(output_dir,  basename .. ".pb-c.c")
		target:add("files", pbSourceFile)
		target:add("includedirs", output_dir, { public = true })
		local need_gen = target:is_rebuilt() or (not os.isfile(pbSourceFile))
		depend.on_changed(function() need_gen = true end, { files = file })
		if need_gen then
			progress.show(opt.progress, "compiling.proto %s", file)
			os.vrunv("protoc", { "--c_out=" .. output_dir, file})
		end
	end)

target("server")
	set_kind("binary")
	add_files("server.c", "helloworld.proto")
	add_rules("grpc-c")
	if is_mode("debug") then
		add_cflags("-gdwarf-4")
	end

target("client")
	set_kind("binary")
	add_files("client.c", "helloworld.proto")
	add_rules("grpc-c")
	if is_mode("debug") then
		add_cflags("-gdwarf-4")
	end
