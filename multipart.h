#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "openflow_1_3.h"
#define MULT_REQ_PORT  sizeof(struct ofp_multipart_request)+sizeof(struct ofp_port_stats_request)+sizeof(struct ofp_match)*6+sizeof(char)*16*6
#define MULT_REQ_FLOW  sizeof(struct ofp_flow_stats_request)+sizeof(struct ofp_port_stats_request)+sizeof(struct ofp_match)*6+sizeof(char)*16*6
#define MULT_REPLY_PORT  sizeof(struct ofp_multipart_reply)+sizeof(struct ofp_port_stats)+sizeof(struct ofp_match)*6+sizeof(char)*16*6
void Aggre_stats_Reply(struct ofp_multipart_reply **mulReplyP,int *flow_length);
void Creat_portstats_Reply(struct ofp_port_stats **port_reply,int port_no,int rx_packets,int tx_packets,int rx_bytes,int tx_bytes,int duration_sec,int duration_nsec);
void Creat_mult_Reply(struct ofp_multipart_reply **mult_ReplyP,int mem_len,int xid,enum ofp_multipart_types type);
void Port_OnuDirect_Init(void);//initional reference proxy port statistics
void Flow_stats_Reply(char* buffer, int buf_len,int sockfd);
void Creat_mult_Req(struct ofp_multipart_request **mult_ReqP,int mem_len,int xid,enum ofp_multipart_types type);
void Creat_Flowstats_Req(struct ofp_flow_stats_request **flow_ReqP,int table_id,int out_port);
int Port_stats_Req_handle(int sockfd,char* buffer,int buf_len,int *flow_length);
int Flow_stats_Req(char* buffer,int buf_len,struct ofp_flow_stats_request **flowPP,int in_port,int out_port);
int Req_port_convert(int port_no);
void multipart_request_handle(char* buffer, int buf_len,int sockfd);
int Reply_port_convert(int port_no);
void multipart_reply_handle(char* buffer, int buf_len,int sockfd);

void New_Port_stats_Reply(); 
