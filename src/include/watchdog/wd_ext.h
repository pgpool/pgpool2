/* -*-pgsql-c-*- */
/*
 *
 * $Header$
 *
 * pgpool: a language independent connection pool server for PostgreSQL
 * written by Tatsuo Ishii
 *
 * Copyright (c) 2003-2012	PgPool Global Development Group
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
extern pid_t wd_ppid;
extern pid_t wd_main(int fork_wait_time);
extern int wd_chk_sticky(void);
extern int wd_is_watchdog_pid(pid_t pid);
extern char *wd_process_name_from_pid(pid_t pid);
extern pid_t wd_reaper_watchdog(pid_t pid, bool restart_child);
extern int wd_chk_setuid(void);
extern void wd_kill_watchdog(int sig);
/* utility function*/
extern int watchdog_thread_create(pthread_t *thread, const pthread_attr_t *attr, void *(*start_routine)(void *), void *arg);

/* wd_child.c */
extern pid_t wd_child(int fork_wait_time);

/* wd_init.c */
extern void wd_init(void);

/* wd_list.c */
extern int wd_set_wd_list(char * hostname, int pgpool_port, int wd_port, char * delegate_ip, struct timeval * tv, int status);
extern int wd_add_wd_list(WdDesc * other_wd); extern int wd_set_wd_info(WdInfo * info);
extern WdInfo * wd_is_exist_master(void);
extern int wd_am_I_oldest(void);
extern int wd_set_myself(struct timeval * tv, int status);
extern WdInfo * wd_is_alive_master(void);

extern WdInfo * wd_get_lock_holder(void);
extern WdInfo * wd_get_interlocking(void);
extern bool wd_are_interlocking_all(void);
extern void wd_set_lock_holder(WdInfo *p, bool value);
extern void wd_set_interlocking(WdInfo *info, bool value);
extern void wd_clear_interlocking_info(void);
extern bool wd_is_contactable_master(void);
extern bool wd_are_contactable_all(void);
extern WdInfo * wd_get_watchdog_info(int num);

/* wd_packet.c */
extern int wd_startup(void);
extern int wd_declare(void);
extern int wd_stand_for_master(void);
extern int wd_notice_server_down(void);
extern int wd_update_info(void);
extern int wd_authentication_failed(int sock);
extern int wd_create_send_socket(char * hostname, int port);
extern int wd_create_recv_socket(int port);
extern int wd_accept(int sock);
extern int wd_send_packet(int sock, WdPacket * snd_pack);
extern int wd_recv_packet(int sock, WdPacket * buf);
extern int wd_escalation(void);
extern int wd_start_recovery(void);
extern int wd_end_recovery(void);
extern int wd_send_failback_request(int node_id);
extern int wd_degenerate_backend_set(int *node_id_set, int count);
extern int wd_promote_backend(int node_id);
extern int wd_set_node_mask (WD_PACKET_NO packet_no, int *node_id_set, int count);
extern int wd_send_packet_no(WD_PACKET_NO packet_no );
extern int wd_send_lock_packet(WD_PACKET_NO packet_no, WD_LOCK_ID lock_id);
extern void wd_calc_hash(const char *str, int len, char *buf);
int wd_packet_to_string(WdPacket *pkt, char *str, int maxlen);

/* wd_ping.c */
extern int wd_is_upper_ok(char * server_list);
extern int wd_is_unused_ip(char * ip);

/* wd_if.c */
extern int wd_IP_up(void);
extern int wd_IP_down(void);
extern int wd_get_cmd(char * buf, char * cmd);

/* wd_lifecheck.c */
extern int is_wd_lifecheck_ready(void);
extern int wd_lifecheck(void);
extern int wd_check_heartbeat(WdInfo * pgpool);
extern int wd_ping_pgpool(WdInfo * pgpool);

/* wd_hearbeat.c */
extern int wd_create_hb_send_socket(WdHbIf * hb_if);
extern int wd_create_hb_recv_socket(WdHbIf * hb_if);
extern void wd_hb_send(int sock, WdHbPacket * pkt, int len, const char * destination, const int dest_port);
extern void wd_hb_recv(int sock, WdHbPacket * pkt);
extern pid_t wd_hb_receiver(int fork_wait_time, WdHbIf * hb_if);
extern pid_t wd_hb_sender(int fork_wait_time, WdHbIf * hb_if);

/* wd_interlock.c */
extern int wd_init_interlock(void);
extern void wd_start_interlock(bool by_health_check, int node_id);
extern void wd_end_interlock(void);
extern void wd_leave_interlock(void);
extern void wd_wait_for_lock(WD_LOCK_ID lock_id);
extern bool wd_am_I_lock_holder(void);
extern bool wd_is_locked(WD_LOCK_ID lock_id);
extern void wd_set_lock(WD_LOCK_ID lock_id, bool value);
extern int wd_unlock(WD_LOCK_ID lock);

#endif /* WD_EXT_H */
