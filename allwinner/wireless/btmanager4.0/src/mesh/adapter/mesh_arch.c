#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ell/ell.h>
#include <pthread.h>
#include <unistd.h>
#include <signal.h>

#include "mesh_db.h"
#include "dbus.h"
#include "bluez/include/error.h"
#include "mesh_internal_api.h"

#define LINUX_MESH_VERSION "V5.54"
#define APP_VERSION "2.0.0"

static pthread_t mesh_main_thread;

static void version_dump()
{
    printf("*****************************************************************************************\n");
    printf("*\tAW MESH Library Ver:%s_%s\t Build Time:%s %s\n",LINUX_MESH_VERSION,APP_VERSION,__DATE__,__TIME__);
    printf("*****************************************************************************************\n");
}

static void mesh_init_clean(void)
{
	struct l_dbus *dbus = app_dbus_get_bus();
	struct mesh_application *mesh_app = mesh_application_get_instance();

	if (!dbus)
		return;

	if (mesh_app)
    {
		l_dbus_object_remove_interface(dbus, mesh_app->path,MESH_APPLICATION_INTERFACE);
		l_dbus_object_remove_interface(dbus, mesh_app->path,L_DBUS_INTERFACE_OBJECT_MANAGER);
		mesh_application_free();
	}

	l_dbus_unregister_interface(dbus, MESH_APPLICATION_INTERFACE);
	l_dbus_unregister_interface(dbus, MESH_ELEMENT_INTERFACE);
	l_dbus_unregister_interface(dbus, MESH_PROVISION_AGENT_INTERFACE);

	l_dbus_destroy(dbus);
	dbus = NULL;
}

static void signal_handler(uint32_t signo, void *user_data)
{
	static bool terminated;

	if (terminated)
		return;

	l_main_quit();
	terminated = true;
}

static void *mesh_thread(void *arg)
{
	struct l_dbus *dbus = NULL;
    struct mesh_application *mesh_app = NULL;
	AwMeshEventCb_t cb = arg;
    uint32_t ret;
	pthread_detach(pthread_self());
    version_dump();
	if (!l_main_init())
		goto final;

    meshd_init();

	l_log_set_stderr();

	if (!app_dbus_init()) {
		goto final;
	}

    dbus = app_dbus_get_bus();

    ret = mesh_application_init(dbus,cb);
	if (ret) {
		goto final;
	}

    mesh_app = mesh_application_get_instance();
    ret = mesh_application_init_dbus(mesh_app);

	if (ret) {
		goto final;
	}

    ret = mesh_element_init_dbus(dbus);

	if (ret) {
		goto final;
	}

    ret = mesh_agent_init_dbus(dbus);

	if (ret) {
		goto final;
	}
    ret = mesh_provisioner_init_dbus(mesh_app);

	if (ret) {
		goto final;
	}

    ret = mesh_db_fetch(mesh_app);

	if (ret) {
		goto final;
	}

	l_main_run_with_signal(signal_handler, NULL);

	l_main_exit();

final:
	mesh_init_clean();
	pthread_exit(NULL);
}

//Internal API
static void mesh_enable_clean(uint32_t role)
{
	struct mesh_application *mesh_app = mesh_application_get_instance();
	struct mesh_agent *mesh_agent = mesh_agent_get_instance();

	l_queue_destroy(mesh_app->elements, mesh_element_free);

	mesh_agent_free(mesh_agent);
	mesh_db_prov_db_free();
}


void mesh_stack_reply(AW_MESH_APP_REQUEST_TYPE_T req_type,const char *readme,int errcode)
{
    AwMeshEventCb_t event_cb = mesh_application_get_event_cb();
    AW_MESH_EVENT_T mesh_event;
    AW_MESH_STACK_REPLY_T *resp = &mesh_event.param.stack_reply;
	mesh_event.evt_code = AW_MESH_STACK_REPLY_STATUS;
    if(event_cb)
    {
        resp->req.type = req_type;
        if(errcode == MESH_ERROR_NONE)
        {
            resp->status = AW_MESH_STACK_REPLY_SUCCESS;
            resp->stack_errcode = MESH_ERROR_NONE;
        }
        else
        {
             resp->status = AW_MESH_STACK_REPLY_FAILED;
             resp->stack_errcode = errcode;
        }
        resp->readme = readme;
        event_cb(&mesh_event,NULL);
    }
}

void mesh_stack_response(AW_MESH_APP_REQUEST_TYPE_T req_type,int errcode,const char *fmt,...)

{
    char buf[AW_MESH_LOG_SIZE] = {'\0'};
    va_list args;
    va_start(args,fmt);
    vsprintf(buf, fmt, args);
    va_end(args);
    mesh_stack_reply(req_type,buf,errcode);
}

int32_t aw_mesh_enable(AW_MESH_ROLE_T role, uint32_t feature, uint8_t *uuid)
{
    struct mesh_db_node *db_node = (struct mesh_db_node *)mesh_db_find_node(uuid);
	struct mesh_application *app = mesh_application_get_instance();
    int32_t ret = AW_MESH_ERROR_NONE;

    MESH_READY_ACCESS(app);
#ifdef PTS_APP_ENABLE
    pts_app_run();
#endif
    if(db_node)
    {
        //if(db_node->role != role)
           // return AW_MESH_ERROR_INVALID_ROLE;

		db_node->role = role;

        if(db_node->attached == 1)
            return AW_MESH_ERROR_ALREADY_ATTACH;

        if (!mesh_application_attach(&db_node->token))
        {
             return AW_MESH_ERROR_ATTACH_FAIL;
        }
        app->provisioner = role;
        db_node->attached = 1;

        return AW_MESH_ERROR_NONE;
    }

    mesh_db_prov_db_init();

    if(role == AW_MESH_ROLE_PROVISIONER)
    {
        ret = mesh_provisioner_create_network(app, uuid);
    }
    else
    {
        ret = mesh_provisionee_join_network(app, uuid);
    }

    if(ret != AW_MESH_ERROR_NONE)
    {
        mesh_enable_clean(role);
    }

	return ret;
}

//Public API
int32_t aw_mesh_init(AwMeshEventCb_t cb)
{
	if((pthread_create(&mesh_main_thread, NULL, mesh_thread, cb) != 0)) {
		return AW_MESH_ERROR_THREAD_CREATE_FAIL;
	}

	while (app_dbus_get_state() != DBUS_INITIALIZED)
		sleep(1);

	return AW_ERROR_NONE;
}

int32_t aw_mesh_deinit(void)
{
	signal_handler(SIGTERM, NULL);
	return AW_ERROR_NONE;
}
