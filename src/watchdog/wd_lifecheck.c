/*
 * $Header$
 *
 * Handles watchdog connection, and protocol communication with pgpool-II
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2013	PgPool Global Development Group
 *
 * Permission to use, copy, modify, and distribute this software and
 * its documentation for any purpose and without fee is hereby
 * granted, provided that the above copyright notice appear in all
 * copies and that both that copyright notice and this permission
 * notice appear in supporting documentation, and that the name of the
 * author not be used in advertising or publicity pertaining to
 * distribution of the software without specific, written prior
 * permission. The author makes no representations about the
 * suitability of this software for any purpose.  It is provided "as
 * is" without express or implied warranty.
 *
 */
#include <pthread.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <netdb.h>
#include "pool.h"
#include "utils/elog.h"
#include "utils/json.h"
#include "utils/json_writer.h"
#include "pool_config.h"
#include "watchdog/watchdog.h"
#include "watchdog/wd_ext.h"

#include "libpq-fe.h"

/*
 * thread argument for lifecheck of pgpool
 */
typedef struct {
	LifeCheckNode *lifeCheckNode;
	int retry;		/* retry times (not used?)*/
} WdPgpoolThreadArg;

static void * thread_ping_pgpool(void * arg);
static PGconn * create_conn(char * hostname, int port);
static int pgpool_down(WdInfo * pool);

static void check_pgpool_status(void);
static void check_pgpool_status_by_query(void);
static void check_pgpool_status_by_hb(void);
static int ping_pgpool(PGconn * conn);
static int is_parent_alive(void);
static bool get_watchdog_nodes(void);
//int g_wd_sock = -1;
static int
wd_check_heartbeat(LifeCheckNode* node);
LifeCheckCluster* gslifeCheckCluster = NULL; /* lives in shared memory */


static LifeCheckNode* get_watchdog_node_from_json(json_value* source);
static void load_watchdog_nodes_from_json(char* json_data, int len);
static bool send_ipc_command_to_socket(int sock, char type, char* data, int len);

bool initialize_lifecheck(void)
{
	int i;
	pid_t hb_receiver_pid[10];
	pid_t hb_sender_pid[10];

	/* get the watchdog node list */
	for (i =0; i < 5; i++)
	{
		if (get_watchdog_nodes() == true)
			break;
		sleep(1);
	}

	if (!gslifeCheckCluster)
		ereport(ERROR,
			(errmsg("unable to initialize lifecheck, watchdog not responding")));

	/* start heartbeat sender and receiver */
	if (!strcmp(pool_config->wd_lifecheck_method, MODE_HEARTBEAT))
	{
		for (i = 0; i < pool_config->num_hb_if; i++)
		{
			/* heartbeat receiver process */
			hb_receiver_pid[i] = wd_hb_receiver(1, &(pool_config->hb_if[i]));

			/* heartbeat sender process */
			hb_sender_pid[i] = wd_hb_sender(1, &(pool_config->hb_if[i]));
		}
	}
	return true;
}

static void print_lifecheck_cluster(void)
{
	int i;
	if (!gslifeCheckCluster)
		return;
	printf("Nodes count = %d\n",gslifeCheckCluster->nodeCount);
	for (i = 0; i< gslifeCheckCluster->nodeCount; i++)
	{
		printf("NODE NO %d\n",i);
		printf("\t ID      = %d\n",gslifeCheckCluster->lifeCheckNodes[i].ID);
		printf("\t Name    = %s\n",gslifeCheckCluster->lifeCheckNodes[i].nodeName);
		printf("\t Host    = %s\n",gslifeCheckCluster->lifeCheckNodes[i].hostName);
		printf("\t WDPort  = %d\n",gslifeCheckCluster->lifeCheckNodes[i].wdPort);
		printf("\t pp Port = %d\n",gslifeCheckCluster->lifeCheckNodes[i].pgpoolPort);
		printf("--------------\n");
	}
	printf("========\n");
}

static bool send_ipc_command_to_socket(int sock, char type, char* data, int len)
{
	int nlen = htonl(len);
	write(sock,&type,1);
	/* command action Default=0*/
	write(sock,&len,4);
	/* data length */
	write(sock,&nlen,4);
	/*read the list */
	if (len > 0)
		write(sock,data,len);
	return true;
}

static bool inform_node_status(LifeCheckNode* node, char *message)
{
	printf("********%s:%d********** node->ID = %d\n",__FUNCTION__,__LINE__,node->ID);
	int sock = open_wd_command_sock(false);
	bool ret;
	if (sock < 0)
	{
		return false;
	}
	printf("********%s:%d**********\n",__FUNCTION__,__LINE__);

	JsonNode* jNode = jw_create_with_object(true);
	/* add the node ID */
	jw_put_int(jNode, "NodeID", node->ID);
	/* add the node status */
	jw_put_int(jNode, "NodeStatus",node->nodeState);
	/* add the node message if any */
	if (message)
		jw_put_string(jNode, "Message", message);

	jw_finish_document(jNode);
	ret = send_ipc_command_to_socket( sock, WD_NODE_STATUS_CHANGE_COMMAND,
							   jw_get_json_string(jNode), jw_get_json_length(jNode));
	jw_destroy(jNode);
	printf("********%s:%d**********\n",__FUNCTION__,__LINE__);

	return ret;
}

static bool get_watchdog_nodes(void)
{
	char type = WD_GET_NODES_LIST_COMMAND;
	int sock = open_wd_command_sock(false);
	int len = 0;
	char* json_data;
	if (sock < 0)
	{
		return false;
	}
	write(sock,&type,1);
	/* command action Default=0*/
	write(sock,&len,4);
	/* data length again 0*/
	write(sock,&len,4);
	/*read the list */
	read(sock, &type, 1);
	read(sock, &len, 4);
	len = ntohl(len);
	if (len > 0)
	{
		json_data = palloc(len +1);
		json_data[len] = 0;
		read(sock, json_data, len);
		ereport(DEBUG2,
				(errmsg("%s",json_data)));
		close(sock);
		load_watchdog_nodes_from_json(json_data,len);
		pfree(json_data);
		return true;
	}
	else
	{
		ereport(ERROR,
				(errmsg("get node list command reply contains no data")));
	}
	return false;
}

static void load_watchdog_nodes_from_json(char* json_data, int len)
{
	json_value* root;
	json_value* value;
	int i,nodeCount;

	root = json_parse(json_data,len);

	/* The root node must be object */
	if (root == NULL || root->type != json_object)
	{
		json_value_free(root);
		ereport(ERROR,
			(errmsg("unable to parse json data for node list")));
	}

	if (json_get_int_value_for_key(root, "NodeCount", &nodeCount))
	{
		json_value_free(root);
		ereport(ERROR,
			(errmsg("invalid json data"),
				 errdetail("unable to find NodeCount node from data")));
	}

	/* find the WatchdogNodes array */
	value = json_get_value_for_key(root,"WatchdogNodes");
	if (value == NULL)
	{
		json_value_free(root);
		ereport(ERROR,
				(errmsg("invalid json data"),
				 errdetail("unable to find WatchdogNodes node from data")));
	}
	if (value->type != json_array)
	{
		json_value_free(root);
		ereport(ERROR,
			(errmsg("invalid json data"),
				 errdetail("WatchdogNodes node does not contains Array")));
	}
	if (nodeCount != value->u.array.length)
	{
		json_value_free(root);
		ereport(ERROR,
			(errmsg("invalid json data"),
				 errdetail("WatchdogNodes array contains %d nodes while expecting %d",value->u.array.length, nodeCount)));
	}
	
	/* get all the Watchdog nodes */
	LifeCheckNode** tmpNodes = palloc(sizeof(LifeCheckNode*) * nodeCount);
	for (i = 0; i < nodeCount; i++)
	{
		tmpNodes[i] = get_watchdog_node_from_json(value->u.array.values[i]);
	}
	json_value_free(root);
	/* okay we are done, put this in shared memory */
	gslifeCheckCluster = pool_shared_memory_create(sizeof(LifeCheckCluster));
	gslifeCheckCluster->nodeCount = nodeCount;
	gslifeCheckCluster->lifeCheckNodes = pool_shared_memory_create(sizeof(LifeCheckNode) * gslifeCheckCluster->nodeCount);
	for (i = 0; i < nodeCount; i++)
	{
		gslifeCheckCluster->lifeCheckNodes[i] = *tmpNodes[i];
//		memcpy(gslifeCheckCluster->lifeCheckNodes, tmpNodes,(sizeof(LifeCheckNode) * gslifeCheckCluster->nodeCount));
//		tmpNodes[i] = get_watchdog_node_from_json(value->u.array.values[i]);
	}
	print_lifecheck_cluster();
	pfree(tmpNodes);
}

static LifeCheckNode* get_watchdog_node_from_json(json_value* source)
{
	char* ptr;
	LifeCheckNode* lifeCheckNode = palloc0(sizeof(LifeCheckNode));
	if (source->type != json_object)
		ereport(ERROR,
			(errmsg("invalid json data"),
				 errdetail("node is not of object type")));

	if (json_get_int_value_for_key(source, "ID", &lifeCheckNode->ID))
	{
		ereport(ERROR,
			(errmsg("invalid json data"),
				 errdetail("unable to find Watchdog Node ID")));
	}

	ptr = json_get_string_value_for_key(source, "NodeName");
	if (ptr == NULL)
	{
		ereport(ERROR,
			(errmsg("invalid json data"),
				 errdetail("unable to find Watchdog Node Name")));
	}
	strncpy(lifeCheckNode->nodeName, ptr, sizeof(lifeCheckNode->nodeName));

	ptr = json_get_string_value_for_key(source, "HostName");
	if (ptr == NULL)
	{
		ereport(ERROR,
			(errmsg("invalid json data"),
				 errdetail("unable to find Watchdog Host Name")));
	}
	strncpy(lifeCheckNode->hostName, ptr, sizeof(lifeCheckNode->hostName));

	if (json_get_int_value_for_key(source, "PgpoolPort", &lifeCheckNode->wdPort))
	{
		ereport(ERROR,
				(errmsg("invalid json data"),
				 errdetail("unable to find WdPort")));
	}

	if (json_get_int_value_for_key(source, "PgpoolPort", &lifeCheckNode->pgpoolPort))
	{
		ereport(ERROR,
			(errmsg("invalid json data"),
				 errdetail("unable to find PgpoolPort")));
	}

	return lifeCheckNode;
}

int
is_wd_lifecheck_ready(void)
{
	int rtn = WD_OK;
	int i;

	for (i = 0; i< gslifeCheckCluster->nodeCount; i++)
	{
		LifeCheckNode* node = &gslifeCheckCluster->lifeCheckNodes[i];
		/* query mode */
		if (!strcmp(pool_config->wd_lifecheck_method, MODE_QUERY))
		{
			if (wd_ping_pgpool(node) == WD_NG)
			{
				ereport(DEBUG1,
					(errmsg("watchdog checking life check is ready"),
						errdetail("pgpool:%d at \"%s:%d\" has not started yet",
							   i, node->hostName, node->pgpoolPort)));
				rtn = WD_NG;
			}
		}
		/* heartbeat mode */
		else if (!strcmp(pool_config->wd_lifecheck_method, MODE_HEARTBEAT))
		{
			if (node->ID == 0) /* local node */
				continue;

			if (!WD_TIME_ISSET(node->hb_last_recv_time) ||
			    !WD_TIME_ISSET(node->hb_send_time))
			{
				ereport(DEBUG1,
					(errmsg("watchdog checking life check is ready"),
						errdetail("pgpool:%d at \"%s:%d\" has not send the heartbeat signal yet",
							   i, node->hostName, node->pgpoolPort)));
				rtn = WD_NG;
			}
		}
		/* otherwise */
		else
		{
			ereport(ERROR,
				(errmsg("checking if watchdog is ready, unkown watchdog mode \"%s\"",
							pool_config->wd_lifecheck_method)));
		}
	}

	return rtn;
}

/*
 * Check if pgpool is living
 */
int
wd_lifecheck(void)
{
	struct timeval tv;

	/* I'm in down.... */
//	if (WD_MYSELF->status == WD_DOWN)
//	{
//		ereport(NOTICE,
//				(errmsg("watchdog lifecheck, watchdog status is DOWN. You need to restart pgpool")));
//		return WD_NG;
//	}

	/* set startup time */
	gettimeofday(&tv, NULL);
	sleep(4);
	/* check upper connection */
//	if (strlen(pool_config->trusted_servers))
//	{
//		if(wd_is_upper_ok(pool_config->trusted_servers) != WD_OK)
//		{
//			ereport(WARNING,
//					(errmsg("watchdog lifecheck, failed to connect to any trusted servers")));
//
//			if (WD_MYSELF->status == WD_MASTER &&
//				strlen(pool_config->delegate_IP) != 0)
//			{
//				wd_IP_down();
//			}
//			wd_set_myself(&tv, WD_DOWN);
//			wd_notice_server_down();
//
//			return WD_NG;
//		}
//	}

	/* skip lifecheck during recovery execution */
	if (*InRecovery != RECOVERY_INIT)
	{
		return WD_OK;
	}

	/* check and update pgpool status */
	check_pgpool_status();

	return WD_OK;
}

/*
 * check and update pgpool status
 */
static void
check_pgpool_status()
{
	/* query mode */
	if (!strcmp(pool_config->wd_lifecheck_method, MODE_QUERY))
	{
		check_pgpool_status_by_query();
	}
	/* heartbeat mode */
	else if (!strcmp(pool_config->wd_lifecheck_method, MODE_HEARTBEAT))
	{
		check_pgpool_status_by_hb();
	}
}

static void
check_pgpool_status_by_hb(void)
{
	int i;
	struct timeval tv;
	LifeCheckNode* node = &gslifeCheckCluster->lifeCheckNodes[0];
	gettimeofday(&tv, NULL);

	/* about myself */
	/* parent is dead so it's orphan.... */
	if (is_parent_alive() == WD_NG && node->nodeState != NODE_DEAD)
	{
		node->nodeState = NODE_DEAD;
		ereport(LOG,
				(errmsg("checking pgpool status by heartbeat"),
					errdetail("lifecheck failed. pgpool (%s:%d) seems not to be working",
						   node->hostName, node->pgpoolPort)));

		inform_node_status(node,"parent process is dead");

//		wd_set_myself(&tv, WD_DOWN);
//		wd_notice_server_down();
	}

	
	for (i = 1; i< gslifeCheckCluster->nodeCount; i++)
	{
		node = &gslifeCheckCluster->lifeCheckNodes[i];
		ereport(DEBUG1,
			(errmsg("watchdog life checking by heartbeat"),
				errdetail("checking pgpool %d (%s:%d)",
					   i, node->hostName, node->pgpoolPort)));

		if (wd_check_heartbeat(node) == WD_NG)
		{
			ereport(LOG,
				(errmsg("checking pgpool status by heartbeat"),
					 errdetail("lifecheck failed. pgpool: %d at \"%s:%d\" seems not to be working",
							   i, node->hostName, node->pgpoolPort)));
			
			if (node->nodeState != NODE_DEAD)
			{
				node->nodeState = NODE_DEAD;
				inform_node_status(node,"No heartbeat signal from node");
			}

//			if (p->status != WD_DOWN)
//				pgpool_down(p);
		}
		else
		{
			ereport(DEBUG1,
				(errmsg("checking pgpool status by heartbeat"),
					 errdetail("OK; status OK")));
		}
	}
}

static void
check_pgpool_status_by_query(void)
{
	pthread_attr_t attr;
	pthread_t thread[MAX_WATCHDOG_NUM];
	WdPgpoolThreadArg thread_arg[MAX_WATCHDOG_NUM];
	LifeCheckNode* node;
	int rc,i;

	/* thread init */
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	/* send queries to all pgpools using threads */
	for (i = 0; i< gslifeCheckCluster->nodeCount; i++)
	{
		node = &gslifeCheckCluster->lifeCheckNodes[i];
		thread_arg[i].lifeCheckNode = node;
		rc = watchdog_thread_create(&thread[i], &attr, thread_ping_pgpool, (void*)&thread_arg[i]);
	}

	pthread_attr_destroy(&attr);

	/* check results of queries */
	for (i = 0; i< gslifeCheckCluster->nodeCount; i++)
	{
		int result;
		node = &gslifeCheckCluster->lifeCheckNodes[i];

		ereport(DEBUG1,
				(errmsg("checking pgpool status by query"),
					errdetail("checking pgpool %d (%s:%d)",
						   i, node->hostName, node->pgpoolPort)));

		rc = pthread_join(thread[i], (void **)&result);
		if ((rc != 0) && (errno == EINTR))
		{
			usleep(100);
			continue;
		}

		if (result == WD_OK)
		{
			ereport(DEBUG1,
				(errmsg("checking pgpool status by query"),
					 errdetail("WD_OK: status: %d", node->nodeState)));

			/* life point init */
			node->retry_lives = pool_config->wd_life_point;
		}
		else
		{
			ereport(DEBUG1,
				(errmsg("checking pgpool status by query"),
					 errdetail("NG; status: %d life:%d", node->nodeState, node->retry_lives)));
			if (node->retry_lives > 0)
			{
				node->retry_lives --;
			}

			/* pgpool goes down */
			if (node->retry_lives <= 0)
			{
				ereport(LOG,
					(errmsg("checking pgpool status by query"),
						errdetail("lifecheck failed %d times. pgpool %d (%s:%d) seems not to be working",
								   pool_config->wd_life_point, i, node->hostName, node->pgpoolPort)));

				if (node->nodeState == NODE_DEAD)
					continue;
				/* It's me! */
				if (i == 0)
					inform_node_status(node,"parent process is dead");
				else
					inform_node_status(node,"unable to connect to node");

				node->nodeState = NODE_DEAD;
//
//				/* It's other pgpool */
//				else if (p->status != WD_DOWN)
//					pgpool_down(p);
			}
		}
	}
}

/*
 * Thread function to send lifecheck query to pgpool
 * Used in wd_lifecheck.
 */
static void *
thread_ping_pgpool(void * arg)
{
	uintptr_t rtn;
	WdPgpoolThreadArg * thread_arg = (WdPgpoolThreadArg *)arg;
	rtn = (uintptr_t)wd_ping_pgpool(thread_arg->lifeCheckNode);

	pthread_exit((void *)rtn);
}

/*
 * Create connection to pgpool
 */
static PGconn *
create_conn(char * hostname, int port)
{
	static char conninfo[1024];
	PGconn *conn;

	if (strlen(pool_config->wd_lifecheck_dbname) == 0)
	{
		ereport(WARNING,
				(errmsg("watchdog life checking, wd_lifecheck_dbname is empty")));
		return NULL;
	}

	if (strlen(pool_config->wd_lifecheck_user) == 0)
	{
		ereport(WARNING,
				(errmsg("watchdog life checking, wd_lifecheck_user is empty")));
		return NULL;
	}

	snprintf(conninfo,sizeof(conninfo),
		"host='%s' port='%d' dbname='%s' user='%s' password='%s' connect_timeout='%d'",
		hostname,
		port,
		pool_config->wd_lifecheck_dbname,
		pool_config->wd_lifecheck_user,
		pool_config->wd_lifecheck_password,
		pool_config->wd_interval / 2 + 1);
	conn = PQconnectdb(conninfo);

	if (PQstatus(conn) != CONNECTION_OK)
	{
		ereport(DEBUG1,
			(errmsg("watchdog life checking"),
				 errdetail("Connection to database failed: %s", PQerrorMessage(conn))));
		PQfinish(conn);
		return NULL;
	}
	return conn;
}

/* handle other pgpool's down */
static int
pgpool_down(WdInfo * pool)
{
	int rtn = WD_OK;
	WD_STATUS prev_status;

	ereport(LOG,
		(errmsg("active pgpool goes down"),
			 errdetail("pgpool on %s:%d down",
					   pool->hostname, pool->pgpool_port)));

	prev_status = pool->status;
	pool->status = WD_DOWN;

	/* the active pgpool goes down and I'm sandby pgpool */
	if (prev_status == WD_MASTER && WD_MYSELF->status == WD_NORMAL)
	{
		if (wd_am_I_oldest() == WD_OK)
		{
			ereport(LOG,
				(errmsg("active pgpool goes down"),
					 errdetail("I am the oldest, so standing for master")));

			/* stand for master */
			rtn = wd_stand_for_master();
			if (rtn == WD_OK)
			{
				/* win */
				wd_escalation();
			}
			else
			{
				/* rejected by others */
				pool->status = prev_status;
			}
		}
	}

	return rtn;
}

/*
 * Check if pgpool is alive using heartbeat signal.
 */
static int
wd_check_heartbeat(LifeCheckNode* node)
{
	int interval;
	struct timeval tv;

	if (!WD_TIME_ISSET(node->hb_last_recv_time) ||
	    !WD_TIME_ISSET(node->hb_send_time))
	{
		ereport(DEBUG1,
			(errmsg("watchdog checking if pgpool is alive using heartbeat"),
				errdetail("pgpool (%s:%d) was restarted and has not send the heartbeat signal yet",
					   node->hostName, node->pgpoolPort)));
		return WD_OK;
	}

	gettimeofday(&tv, NULL);

	interval = WD_TIME_DIFF_SEC(tv, node->hb_last_recv_time);
	ereport(DEBUG1,
		(errmsg("watchdog checking if pgpool is alive using heartbeat"),
			errdetail("the last heartbeat from \"%s:%d\" received %d seconds ago",
				   node->hostName, node->pgpoolPort, interval)));

	if (interval > pool_config->wd_heartbeat_deadtime)
	{
		return WD_NG;
	}

	if (node->nodeState == NODE_DEAD)
	{
		node->nodeState = NODE_ALIVE;
		inform_node_status(node,"Heartbeat signal found");
	}
	return WD_OK;
}

/*
 * Check if pgpool can accept the lifecheck query.
 */
int
wd_ping_pgpool(LifeCheckNode* node)
{
	PGconn * conn;

	conn = create_conn(node->hostName, node->pgpoolPort);
	if (conn == NULL)
		return WD_NG;
	return ping_pgpool(conn);
}

/* inner function for issueing lifecheck query */
static int
ping_pgpool(PGconn * conn)
{
	int rtn = WD_NG;
	int status = PGRES_FATAL_ERROR;
	PGresult * res = (PGresult *)NULL;

	if (!conn)
	{
		return WD_NG;
	}

	res = PQexec(conn, pool_config->wd_lifecheck_query );

	status = PQresultStatus(res);
	if (res != NULL)
	{
		PQclear(res);
	}

	if ((status != PGRES_NONFATAL_ERROR ) &&
		(status != PGRES_FATAL_ERROR ))
	{
		rtn = WD_OK;
	}
	PQfinish(conn);

	return rtn;
}

static int
is_parent_alive()
{
	if (mypid == getppid())
		return WD_OK;
	else
		return WD_NG;
}
