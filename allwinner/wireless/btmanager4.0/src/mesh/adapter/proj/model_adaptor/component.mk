#
# Component Makefile
#
MODEL_ADAPTOR_SRC := adapter/proj/model_adaptor/src
MODEL_ADAPTOR_INCLUDE := adapter/proj/model_adaptor/include
INCLUDES += -I $(MODEL_ADAPTOR_INCLUDE)
SRC += $(MODEL_ADAPTOR_SRC)/mesh_model_adaptor.c \
       $(MODEL_ADAPTOR_SRC)/mesh_model_adaptor_generic_onoff_client.c \
       $(MODEL_ADAPTOR_SRC)/mesh_model_adaptor_generic_onoff_server.c
