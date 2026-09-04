add_rules("mode.debug", "mode.release")

add_requires("protobuf")
add_requires("grpc++")

rule("grpc")
	set_extensions(".proto")
	on_prepare_file(function(target, file, opt)
		import("core.project.depend")
		import("utils.progress")
		import("lib.detect.find_file")
		target:add("packages", "grpc++")
		local output_dir = target:autogendir()
		os.mkdir(output_dir)
		local basename = path.basename(file)
		local pbSourceFile = path.join(output_dir,  basename .. ".pb.cc")
		local rpcSourceFile = path.join(output_dir, basename .. ".grpc.pb.cc")
		target:add("files", pbSourceFile, rpcSourceFile)
		target:add("includedirs", output_dir, { public = true })
		local pluginName = "grpc_cpp_plugin"
		if os.is_host("windows") then
			pluginName = pluginName .. ".exe"
		end
		local plg = find_file(pluginName, { "$(env PATH)" })
		if plg == nil then
			cprint("${red} not found Tool:", pluginName)
		end
		local need_gen = target:is_rebuilt() or (not os.isfile(pbSourceFile)) or (not os.isfile(rpcSourceFile))
		depend.on_changed(function() need_gen = true end, { files = file })
		if need_gen then
			progress.show(opt.progress, "compiling.proto %s", file)
			os.vrunv("protoc",
				{ "--cpp_out=" .. output_dir, "--grpc_out=" .. output_dir, "--plugin=protoc-gen-grpc=" .. plg, file })
		end
	end)

target("server")
	set_kind("binary")
	add_files("server/*.cc")
	add_files("*.proto")
	add_rules("grpc")
	add_packages("protobuf")
	if is_mode("debug") then
		add_cxflags("-gdwarf-4")
	end

target("client")
	set_kind("binary")
	add_files("client/*.cc")
	add_files("*.proto")
	add_rules("grpc")
	add_packages("protobuf")
	if is_mode("debug") then
		add_cxflags("-gdwarf-4")
	end
