cmake_minimum_required(VERSION 3.10)

# 项目名称
project(TruthRestoration)

# 汇编文件路径
set(BOOT_DIR "真相还原/c5/boot")
set(INCLUDE_DIR "真相还原/c5/include")
set(BUILD_DIR "真相还原/c5/build")

# 确保构建目录存在
file(MAKE_DIRECTORY ${BUILD_DIR})

# 汇编文件
set(SOURCE_FILES
    ${BOOT_DIR}/loader.S
    ${BOOT_DIR}/mbr.S
)

# 添加汇编目标
foreach(SRC ${SOURCE_FILES})
    get_filename_component(NAME ${SRC} NAME_WE)
    add_custom_command(
        OUTPUT ${BUILD_DIR}/${NAME}.bin
        COMMAND nasm -I ${INCLUDE_DIR} -o ${BUILD_DIR}/${NAME}.bin ${SRC}
        DEPENDS ${SRC}
        COMMENT "Assembling ${SRC}"
    )
endforeach()

# 创建一个目标来生成映像文件
add_custom_target(create_image ALL
    DEPENDS ${BUILD_DIR}/loader.bin ${BUILD_DIR}/mbr.bin
    COMMAND dd if=${BUILD_DIR}/loader.bin of=./HD60.img bs=512 count=2 seek=2 conv=notrunc
    COMMAND dd if=${BUILD_DIR}/mbr.bin of=./HD60.img bs=512 count=1 conv=notrunc
    COMMENT "Creating HD60.img"
)

# 添加 Bochs 的运行目标
add_custom_target(run_bochs
    COMMAND bochs -f bochsrc.disk -q
    DEPENDS create_image
    COMMENT "Running Bochs"
)