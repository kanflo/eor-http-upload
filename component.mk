# Component makefile for extras/http-upload

INC_DIRS += $(http-upload_ROOT)

# args for passing into compile rule generation
http-upload_SRC_DIR =  $(http-upload_ROOT)

$(eval $(call component_compile_rules,http-upload))

