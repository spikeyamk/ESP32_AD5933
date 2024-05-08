import os
import subprocess

def header(out_path):
    with open(out_path, "w") as file:
        file.write("#pragma once\n")
        file.write("\n")
        file.write("#include <string_view>\n")
        file.write("#include <array>\n")
        file.write("\n")
        file.write("namespace Legal {\n")
        file.write("    struct License {\n")
        file.write("        const std::string_view name;\n")
        file.write("        const std::string_view text;\n")
        file.write("    };\n")
        file.write(f"    extern const std::array<const License, {len(submodule_paths)}> licenses;\n")
        file.write("}")

def source(out_path, submodule_paths):
    with open(out_path, "w") as out_file:
        out_file.write("#include \"legal/legal.hpp\"\n")
        out_file.write("\n")
        out_file.write("namespace Legal {\n")

        for (i, path) in enumerate(submodule_paths):
            license_file_path = [os.path.join(path, subpath) for subpath in os.listdir(path) if os.path.isfile(os.path.join(path, subpath)) and "license" in subpath.lower()][0]
            with open(license_file_path, "rb") as license_file:
                byte_values = license_file.read()
                hex_values = [hex(byte_value) for byte_value in byte_values]
                formatted_hex_values = ", ".join([f"0x{value[2:].zfill(2)}" for value in hex_values])
                out_file.write(f"    const char const_char_pointer_{i}[] ")
                out_file.write("{\n")
                out_file.write(f"        {formatted_hex_values}\n")
                out_file.write("    };\n")

        out_file.write(f"    const std::array<const License, {len(submodule_paths)}> licenses ")
        out_file.write("{\n")

        for (i, path) in enumerate(submodule_paths):
            out_file.write("        License {\n")
            out_file.write("            {\n")
            out_file.write(f"                \" {os.path.basename(path)}\"\n")
            out_file.write("            },\n")
            out_file.write("            {\n")
            out_file.write(f"                const_char_pointer_{i}\n")
            out_file.write("            },\n")
            out_file.write("        },\n")

        out_file.write("    };\n")
        out_file.write("}")

submodule_command_output = subprocess.run("git submodule", stdout=subprocess.PIPE).stdout.decode('utf-8')
submodule_paths = [os.path.abspath(line.split()[1]) for line in submodule_command_output.split('\n') if line.strip()]

header("include/legal/legal.hpp")
source("lib/legal.cpp", submodule_paths)