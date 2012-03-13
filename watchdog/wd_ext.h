/* -*-pgsql-c-*- */
/*
 *
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL 
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2011	PgPool Global Development Group
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
 * wd_ext.h.: watchdog extern definition header file
 *
 */

#ifndef WD_EXT_H
#define WD_EXT_H
/* watchdog.c */
extern int wd_main(int fork_wait_time);

/* wd_child.c */
extern int wd_child(int fork_wait_time);

/* wd_init.c */
extern int wd_init(void);

/* wd_list.c */
extern int wd_set_wd_list(char * hostname, int pgpool_port, int wd_port, struct timeval * tv, int status);
extern int wd_add_wd_list(WdDesc * other_wd);
extern int wd_set_wd_info(WdInfo * info);
extern WdInfo * wd_is_exist_master(void);
extern int wd_am_I_oldest(void);
extern int wd_set_myself(struct timeval * tv, int status);

/* wd_packet.c */
extern int wd_startup(void);
extern int wd_declare(void);
extern int wd_stand_for_master(void);
extern int wd_notice_server_down(void);
extern int wd_create_send_socket(char * hostname, int port);
extern int wd_create_recv_socket(int port);
extern int wd_accept(int sock);
extern int wd_send_packet(int sock, WdPacket * snd_pack);
extern int wd_recv_packet(int sock, WdPacket * buf);
extern int wd_escalation(void);

/* wd_ping.c */
extern int wd_is_upper_ok(char * server_list);

/* wd_if.c */
extern int wd_IP_up(void);
extern int wd_IP_down(void);

/* wd_lifecheck.c */
extern int wd_lifecheck(void);

#endif /* WD_EXT_H */
