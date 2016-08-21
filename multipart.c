#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include"channel_communication.h"
#include "flow.h"
//#include "proxy_table.h"
#include "multipart.h"
#include "oxm_match.h"
#include "list.h"
extern struct virtual_onu_olt_port *pOnutab;
static int t_port_no,cntlfd;
static unsigned int output_count=0,miss_conut=0;;
static unsigned int Port_key=0,Flow_key=0;
int port_no,rx_packet,tx_packet,rx_byte,tx_byte,duration_sec,duration_nsec;
void Aggre_stats_Reply(struct ofp_multipart_reply **mulReplyP,int *flow_length)
{
    printf("---------Aggre_stats_Reply------------\n\n" );

    struct ofp_aggregate_stats_reply  *agg_stats;
    struct ofp_multipart_reply *mulReply,*Create_mulReply;
    struct ofp_port_stats *port_reply;
    
    mulReply = *mulReplyP;
    agg_stats= (struct ofp_aggregate_stats_reply *)mulReply->body;
    Creat_mult_Reply(&Create_mulReply,MULT_REPLY_PORT,htonl(Create_mulReply->header.xid),OFPMP_PORT_STATS);
    port_reply = (struct ofp_port_stats *)Create_mulReply->body;
               printf("The ,agg_stats->packet_count=%d,agg_stats->byte_count=%d\n\n",htonll(agg_stats->packet_count),htonll(agg_stats->byte_count));
    Creat_portstats_Reply(&port_reply,t_port_no,agg_stats->packet_count,agg_stats->packet_count,agg_stats->byte_count,agg_stats->byte_count,1,2);//Have problem
    Create_mulReply->header.length = htons(sizeof(struct ofp_multipart_reply))+htons(sizeof(struct ofp_port_stats));
    *flow_length = htons(Create_mulReply->header.length);
    *mulReplyP = Create_mulReply;
}//Aggre_stats_Reply_handle

void Creat_portstats_Reply(struct ofp_port_stats **port_reply,int port_no,int rx_packets,int tx_packets,int rx_bytes,int tx_bytes,int duration_sec,int duration_nsec)
{
    printf("--------- Creat_portstats_Reply------------\n\n" );
    struct ofp_port_stats *port_stat; 
    port_stat = *port_reply;
   	port_stat->port_no = htonl(port_no);
	port_stat->rx_packets = htonll(rx_packets);     // Number of received packets.
	port_stat->tx_packets = htonll(tx_packets);     // Number of transmitted packets.
	port_stat->rx_bytes = htonll(rx_bytes);       // Number of received bytes.
	port_stat->tx_bytes = htonll(tx_bytes);       // Number of transmitted bytes.
	port_stat->rx_dropped = htonll(4);     // Number of packets dropped by RX.
	port_stat->tx_dropped = htonll(5);     // Number of packets dropped by TX.
	port_stat->rx_errors = htonll(0);      // Number of receive errors.  This is a super-set of more specific receive errors and should be greater than or equal to the sum of all rx_*_err values.
	port_stat->tx_errors = htonll(0);      // Number of transmit errors.  This is a super-set of more specific transmit errors and should be greater than or equal to the sum of all tx_*_err values (none currently defined.)
	port_stat->rx_frame_err = htonll(0);   // Number of frame alignment errors.
	port_stat->rx_over_err = htonll(0);    // Number of packets with RX overrun.
	port_stat->rx_crc_err = htonll(0);     // Number of CRC errors.
	port_stat->collisions = htonll(0);     // Number of collisions.
    port_stat->duration_sec= htonl(duration_sec); /* Time port has been alive in seconds. */
    port_stat->duration_nsec= htonl(duration_nsec); /* Time port has been alive in nanoseconds beyond
duration_sec. */  
}//Creat_portstats_Reply

void Creat_mult_Reply(struct ofp_multipart_reply **mult_ReplyP,int mem_len,int xid,enum ofp_multipart_types type)
{
     printf("--------- Creat_mult_Reply------------\n\n" );
     char *creat_flow;
     //int length = MULT_REQ_PORT ;
     struct ofp_multipart_reply *mult_Reply;

     creat_flow =(char*)malloc(mem_len);
     memset(creat_flow, 0,mem_len);
     
     *mult_ReplyP = (struct ofp_multipart_reply*)creat_flow;
      mult_Reply = *mult_ReplyP;
      mult_Reply->header.version = OFP_VERSION;
      mult_Reply->header.type	   = OFPT_MULTIPART_REPLY;
      mult_Reply->header.length  = htons(sizeof(struct ofp_multipart_reply));
      mult_Reply->header.xid	   = htonl(xid);	
      mult_Reply->type           = htons(type);
      mult_Reply->flags          = htons(OFPMPF_REPLY_MORE);
}//Creat_mult_Reply

void Port_OnuDirect_Init(void)
{
  port_no = 0;
  rx_packet = 0;
  tx_packet = 0;
  rx_byte = 0;
  tx_byte = 0;
  duration_sec = 0;
  duration_nsec = 0;       
}//Port_OnuDirect_Init

void New_Port_stats_Reply() 
{
   printf("--------- New_Port_stats_Reply------------\n\n" );

   struct ofp_multipart_reply *mulReply,*Create_mulReply;
   struct ofp_port_stats *port_reply;
   int flow_len=0;

	Creat_mult_Reply(&Create_mulReply,MULT_REPLY_PORT,htonl(mulReply->header.xid),OFPMP_PORT_STATS);
   port_reply = (struct ofp_port_stats *)Create_mulReply->body;

	printf("The port_no=%d,rx_packet=%d,tx_packe=%d,rx_byte=%d,tx_byte=%d,duration_sec=%d,duration_nsec=%d\n\n",port_no,rx_packet,tx_packet,rx_byte,tx_byte,duration_sec,duration_nsec);

	Creat_portstats_Reply(&port_reply,port_no,rx_packet,tx_packet,rx_byte,tx_byte,duration_sec,duration_nsec);

	Create_mulReply->header.length = htons(sizeof(struct ofp_multipart_reply))+htons(sizeof(struct ofp_port_stats));
	flow_len = htons(Create_mulReply->header.length);
	printf("----flow_len :%d-----\n",flow_len);              
           send(cntlfd,Create_mulReply,flow_len,0);
           free(Create_mulReply);
 printf("--------- New_Port_stats_Reply End------------\n\n" );
}//New_Port_stats_Reply

void Flow_stats_Reply(char *buffer,int buf_len,int sockfd)
{
  printf("--------- Flow_stats_Reply------------\n\n" );
  int in_port,vlan;
  int len_c=0,all_len;
  char * pbuffer; 
   
  struct ofp_multipart_reply *mulReply;
  struct ofp_flow_stats *flowReply; 
 
  struct ofp_match *match;

  mulReply= (struct ofp_multipart_reply *)buffer;
  flowReply = (struct ofp_flow_stats *)mulReply->body;
  printf("mulReply->length:%d\n\n",htons(mulReply->header.length));
  printf("mulReply->type:%d\n",htons(mulReply->type));
  all_len=(htons(mulReply->header.length)-16);
  printf("all_len=:%d\n",all_len);

 while(len_c< all_len){     
  len_c+=htons(flowReply->length);
  printf("flowReply_length:%d\n,len_c:%d\n,flowReply->length:%d\n,all_len=:%d\n\n",buf_len,len_c,htons(flowReply->length),all_len);

  printf("\n This is flow_stats info.....\n\n");
  printf("flowReply->length:%d\n\n",htons(flowReply->length));
  in_port = read_match(&flowReply->match,OFPXMT_OFB_IN_PORT); 
  printf("flow_stats_reply -> in_port :%d\n",in_port); 
  printf("The flowReply->packet_count=%d ,flowReply->byte_count=%d\n\n",htonll(flowReply->packet_count),htonll(flowReply->byte_count)); 
  if(Port_key==1)
   {
    printf("\n This is port_stats info.....\n\n");

    match = &flowReply->match;
    vlan = read_match(match,OFPXMT_OFB_VLAN_VID);
    printf("vlan:%d\n",vlan);             

    if(flowReply->table_id==0 && vlan!=-1 && Port_key==1)
    {
          printf("flowReply->table_id:%d.....\n",flowReply->table_id);
       
          port_no=vlan-1;
          rx_packet = htonll(flowReply->packet_count);
          rx_byte = htonll(flowReply->byte_count);
          duration_sec = htonl(flowReply->duration_sec);
          duration_nsec = htonl(flowReply->duration_nsec);
	  miss_conut=0; 
           printf("The port_no=%d\n,rx_packet=%d,rx_byte=%d\n,duration_sec=%d\n,duration_nsec=%d\n\n",port_no,rx_packet,rx_byte,duration_sec,duration_nsec);                              
    }else{
       printf("flowReply->table_id:%d\n\n",flowReply->table_id);   
       if(output_count!=0)
         {
	    printf("\n---- if(output_count!=0)-----\n\n");
            tx_packet += htonll(flowReply->packet_count);  
            tx_byte += htonll(flowReply->byte_count);
        printf("The tx_packet=%d,tx_byte=%d\n,flowReply->packet_count=%d\n,flowReply->byte_count=%d\n\n",tx_packet,tx_byte,htonll(flowReply->packet_count),htonll(flowReply->byte_count)); 
            output_count--;
	    miss_conut=0;
	    printf("---- output_count:%d-----\n", output_count);              
         }
      if(output_count==0)
	 {
	   printf("\n---- if(output_count!=0)-----\n\n");
           New_Port_stats_Reply(); 
   	   Port_OnuDirect_Init();     
           Port_key=0;                           
          } //else(output_count==0)           
    }//else table_id==1                 
   }//if(Port_key==1)
  else
   {                    
      if(flowReply->table_id==0)
      {
        printf("flowReply->table_id==0,that flow from OFSW\n");                                 
        if(in_port>OF_OLT_CONNT_NUM )
          { 
  	        printf("in_port>OF_OLT_CONNT_NUM .....\n\n");
  	        in_port = of_virtual_port(in_port);                                       
            modify_match(&flowReply->match,OFPXMT_OFB_IN_PORT,in_port);
          }  
      }else{
	if(in_port>0){
            printf("flowReply->table_id:%d\n\n",flowReply->table_id);
            in_port =  flowReply->table_id-1; 
	    printf("in_port:%d\n",in_port);                                                   
            modify_match(&flowReply->match,OFPXMT_OFB_IN_PORT,in_port);    
            flowReply->table_id=0;
	}else{
	    printf("The flowReply is error .....\n\n");
	    return;
	}//else            
      }//else flowReply->table_id==1 
	if(len_c< all_len && Flow_key==1){
      	   send(sockfd,mulReply,buf_len,0);
	   Flow_key=0;
	}//if
                  
    }//else Port_key==0
     pbuffer = (char*)flowReply;
     flowReply =(struct ofp_flow_stats *)(pbuffer + htons(flowReply->length));
  }//while    
}//Flow_stats_Reply

void Creat_mult_Req(struct ofp_multipart_request **mult_ReqP,int mem_len,int xid,enum ofp_multipart_types type)
{
     printf("--------- Creat_mult_Req------------\n\n" );
     char *creat_flow;
     //int length = MULT_REQ_PORT ;
     struct ofp_multipart_request *mult_Req;

     creat_flow =(char*)malloc(mem_len);
     memset(creat_flow, 0,mem_len);
     
     *mult_ReqP = (struct ofp_multipart_request*)creat_flow;
      mult_Req = *mult_ReqP;
      mult_Req->header.version = OFP_VERSION;
      mult_Req->header.type	   = OFPT_MULTIPART_REQUEST;
      mult_Req->header.length  = htons(sizeof(struct ofp_multipart_request));
      mult_Req->header.xid	   = htonl(xid);	
      mult_Req->type           = htons(type);
      mult_Req->flags          = htons(OFPMPF_REQ_MORE);
}//Creat_flow_stats_Req

void Creat_Flowstats_Req(struct ofp_flow_stats_request **flow_ReqP,int table_id,int out_port)
{
     printf("--------- Creat_Flowstats_Req-----------\n\n" );
     struct ofp_flow_stats_request *flow_Req;
     flow_Req = *flow_ReqP;
     flow_Req->table_id = table_id ;
     flow_Req->out_port = htonl(out_port);
     flow_Req->out_group =htonl(OFPG_ANY);
     flow_Req->cookie	 = 0x00ULL;
     flow_Req->cookie_mask	 = 0x00ULL;
     flow_Req->match.type	 = htons(OFPMT_OXM);
     flow_Req->match.length = htons(sizeof(struct ofp_match));
}//Flow_port_stats

int Port_stats_Req_handle(int sockfd,char* buffer,int buf_len,int *flow_length)
{
   printf("---------Port_stats_Req ------------\n\n" );
   //printf("The out_port: %d\n\n",out_port);
   int state,vlan,type,in_port,portno,flow_len; 
   
       /*if port_no <= ONU_PORT_NUM used.*/ 
   struct list *now=NULL;
   int count=0;  
   char *matchP;
   struct ofp_match *match;
   //enum flow_direct_type direct;
   struct ofp_multipart_request  *mulCreate,*mulReq;
   struct ofp_port_stats_request *portP;
   struct ofp_flow_stats_request *flowReq;
   mulReq = (struct ofp_multipart_request *)buffer;
   portP =  (struct ofp_port_stats_request *)mulReq->body;
   portno = Req_port_convert(htonl(portP->port_no));     
   printf("\nThe command port_no:%d,and convert portno:%d\n\n",htonl(portP->port_no),portno );		

   if((htonl(portP->port_no) == OFPP_ANY)){ 
       printf("The portReq->port_no  == OFPP_ANY\n");
       type = OFSW_INPUT;                                                 
   }else if(htonl(portP->port_no) <= ONU_PORT_NUM && Port_key == 0){ 
       t_port_no = htonl(portP->port_no); 
                                 /*use flow_statist getting rx_packet,and  the total amount of flow_statist at table_0 is equal to virtual port*/                   
       Creat_mult_Req(&mulCreate,MULT_REQ_FLOW,htonl(mulReq->header.xid),OFPMP_FLOW);
       flowReq =(struct ofp_flow_stats_request*)mulCreate->body;
       in_port = htonl(portP->port_no);
       printf("The command in_port:%d\n",in_port);
       state = match_inport_covert(&in_port,&vlan,pOnutab);
       printf("The  match_inport_covert:%d,vlan:%d\n",in_port,vlan);
       if(state == OFSW_INPUT)
              return OTHER_ERROR;              
       Creat_Flowstats_Req(&flowReq,0,OFPP_ANY);
              //printf("The Creat_Flowstats_Req set table_id: %d\n\n",flowReq->table_id);    
       Flow_Match(&flowReq->match,OXM_OF_IN_PORT,in_port);
       Flow_Match(&flowReq->match,OXM_OF_VLAN_VID,vlan);
	printf("\nThe Match command portno:%d,and vlan:%d\n\n",portno,vlan);
       mulCreate->header.length = htons(sizeof(struct ofp_multipart_request ))+htons(sizeof(struct ofp_flow_stats_request))-htons(sizeof(struct ofp_match))+flowReq->match.length+htons(OFP_MATCH_OXM_PADDING(htons(flowReq->match.length)));
       flow_len = htons(mulCreate->header.length);
	printf("The flow_len : %d\n\n",flow_len);
       //*mulReqP = mulCreate;
       send(sockfd,mulCreate,flow_len,0);
       flowReq = NULL;
       free(mulCreate);

	print_match_tmp_table();
                              /*use flow_statist get tx_packet*/
       output_count = count_match_entry(htonl(portP->port_no));
       printf("The output_count: %d\n\n",output_count);
       /*if(output_count>2)
		output_count=2;*/
       while (output_count>count){
     	     Creat_mult_Req(&mulCreate,MULT_REQ_FLOW,htonl(mulReq->header.xid)+count,OFPMP_FLOW);
       	     flowReq =(struct ofp_flow_stats_request*)mulCreate->body;
 	     Creat_Flowstats_Req(&flowReq,0,OFPP_ANY);
             matchP = cache_match_data(&now,htonl(portP->port_no));
             match = (struct ofp_match *)matchP;
             memcpy(&flowReq->match,match,htons(match->length));
             printf("The htons(match->length): %d\n\n",htons(match->length));

	    /*check flow direction*/
	     in_port = read_match(match,OFPXMT_OFB_IN_PORT);
             printf("cache_match_data in_port :%d\n\n",in_port);	
             if(in_port>ONU_PORT_NUM) 
		in_port=of_init_port(in_port);
	     else{
       		state = match_inport_covert(&in_port,&vlan,pOnutab);
       		printf("The  match_inport_covert:%d,vlan:%d\n",in_port,vlan);
		flowReq->table_id = vlan;
	     }
	     modify_match(&flowReq->match,OFPXMT_OFB_IN_PORT,in_port);
             mulCreate->header.length = htons(sizeof(struct ofp_multipart_request ))+htons(sizeof(struct ofp_flow_stats_request))-htons(sizeof(struct ofp_match))+flowReq->match.length+htons(OFP_MATCH_OXM_PADDING(htons(flowReq->match.length)));
             flow_len = htons(mulCreate->header.length);
	     printf("The flow_len : %d\n\n",flow_len);
             send(sockfd,mulCreate,flow_len,0);
             flowReq = NULL;
             free(mulCreate);
             count++;
 	printf("The output_count:%d,count++=%d\n",output_count,count++);
        Port_key=1;
        type = ONU_INPUT; 
        Port_OnuDirect_Init();
       } //whlie

       Port_key=1;
       type = ONU_INPUT; 
       Port_OnuDirect_Init();
   }else if(htonl(portP->port_no) <= ONU_PORT_NUM && Port_key != 0){
	    miss_conut++;
	    if(miss_conut>output_count+2){ 
           	New_Port_stats_Reply(); 
   	  	Port_OnuDirect_Init();     
	 	Port_key = 0;
 		miss_conut=0;
	    }	
	    printf("The Message delay,that is pass!(%d)\n\n",miss_conut);

		printf("The port_no=%d,rx_packet=%d,tx_packe=%d,rx_byte=%d,tx_byte=%d,duration_sec=%d,duration_nsec=%d\n\n",port_no,rx_packet,tx_packet,rx_byte,tx_byte,duration_sec,duration_nsec);	
   }else{     
       portP->port_no = htonl(portno); 
       send(sockfd,mulReq,buf_len,0);
       type = OFSW_INPUT;     
   }//else    	
   return type;
}//Port_stats_Req_handle

int Flow_stats_Req(char* buffer,int buf_len,struct ofp_flow_stats_request **flowPP,int in_port,int out_port)
{
   printf("---------Flow stats_Req ------------\n\n" );
   printf("The in_port:%d,out_port: %d\n\n",in_port,out_port);
   int state,vlan,type;  
   enum flow_direct_type direct;
   struct ofp_flow_stats_request *flowP;
   char *creat_flow;

   direct= Flow_direct(in_port,out_port);	
   Flow_key=1;     
   switch(direct) {
 	case NOT_OUTPUT:{
			printf("The case NOT_OUTPUT \n" );	
            state = match_inport_covert(&in_port,&vlan,pOnutab); 
            flowP = *flowPP;     
            if(state== OFSW_INPUT) 
            {   
                modify_match(&flowP->match,OFPXMT_OFB_IN_PORT,in_port);
                //send(sockfd,flowP,buf_len,0);
                type = OFSW_INPUT;
            }    
            else
            {
               /* creat_flow =(char*)malloc(buf_len+sizeof(char)*8);
                memset(creat_flow, 0,buf_len);	
                memcpy(creat_flow,buffer,buf_len);
                flowP  = (struct ofp_flow_stats_request *)creat_flow;
                Flow_Match(&flowP->match,OFPXMT_OFB_IN_PORT,in_port);
                Flow_Match(&flowP->match,OFPXMT_OFB_VLAN_VID,vlan); */
               /* buf_len = buf_len+sizeof(char)*6;
                send(sockfd,flowP,buf_len,0);
                free(flowP);*/ 
		printf("The vlan:%d\n\n",vlan);
                modify_match(&flowP->match,OFPXMT_OFB_IN_PORT,in_port);
                flowP->table_id = vlan;
                type = ONU_INPUT;   
            }   
            *flowPP = flowP;                 
			break;
		}                    
	case U_ONU_TO_OFSW:{
			printf("The case U_ONU_TO_OFSW\n" );	
			break;
		}
	case D_OFSW_TO_ONU:{
			printf("The D_OFSW_TO_ONU \n" );			
			break;
		}
	case UL_OFSW_TO_OFSW:{
			printf("The case UL_OFSW_TO_OFSW \n" );			
			break;
		}
	case DL_ONU_TO_ONU:{
			printf("The case DL_ONU_TO_ONU \n" );			
			break;
		}
	default:{
			printf("Flow stats_Req error\n" );
			break;
		}
   }//switch	
   return type;
}//Flow stats_Req

int Req_port_convert(int port_no)
{
    printf("The Requst_port_convert from port_no: %d\n",port_no );
    int port;
    
    if(port_no  == OFPP_ANY){
       port =OTHER;        
       printf("The convert port_no  == OFPP_ANY\n");         
    }else if( port_no <= ONU_PORT_NUM){
        port = olt_of_port_convert(port_no);
	printf("The olt_of_port_convert convert to : %d\n",port);
        //port =ONU_INPUT;
    }else{
        port = of_init_port(port_no);       
        printf("The Requst_port_no convert to : %d\n",port);
    }          
        return  port;      
}//Req_port_convert

void multipart_request_handle(char* buffer, int buf_len,int sockfd)
{
    printf("------------------Staring handle 'multipart_request' message from controller-------------\n\n");
    struct ofp_multipart_request *mulReq;
    mulReq = (struct ofp_multipart_request *)buffer;
    
    struct ofp_flow_stats_request *flowReq;
    struct ofp_aggregate_stats_request *aggrReq;
    struct ofp_port_stats_request *portReq;
    struct ofp_queue_stats_request *queueReq;

    int portno;
    int in_port,flow_length,type;
    
   switch(htons(mulReq->type)){
     case OFPMP_FLOW:{
             printf("The case  ofp_flow_stats_request \n" ); 
             flowReq = (struct ofp_flow_stats_request*)mulReq->body;
             //flowReq->table_id = 0;
             //flowReq->out_port = htonl(OFPP_ANY);
             in_port = read_match(&flowReq->match,OFPXMT_OFB_IN_PORT);
             printf("flow_stats_request -> in_port :%d\n\n",in_port);
             printf("---flowReq->match.length=%d---\n\n",htons(flowReq->match.length));
             type = Flow_stats_Req(buffer,buf_len,&flowReq,in_port,htonl(flowReq->out_port));
            /* printf("---New_flowReq->match.length=%d---\n\n",htons(flowReq->match.length));
             printf("---mulReq->header.length=%d---\n",htons(mulReq->header.length));
             mulReq->header.length = htons(sizeof(struct ofp_multipart_request ))+htons(sizeof(struct ofp_flow_stats_request))-htons(sizeof(struct ofp_match))+flowReq->match.length+htons(OFP_MATCH_OXM_PADDING(htons(flowReq->match.length)));
             printf("---NEW_mulReq->header.length=%d---\n\n",htons(mulReq->header.length));
             flow_length = htons(mulReq->header.length);
             send(sockfd,mulReq,flow_length,0);
             if(type==ONU_INPUT)
               free(mulReq);   */   
             send(sockfd,mulReq,buf_len,0);   
             break;
          }
     case OFPMP_AGGREGATE:{
             printf("The case  ofp_flow_stats_request \n" );
             aggrReq = (struct ofp_aggregate_stats_request*)mulReq->body; 
             aggrReq->table_id =0;
             //aggrReq->out_port = htonl(OFPP_ANY); 
             send(sockfd,mulReq,buf_len,0);                
             break;
          }   
     case OFPMP_PORT_STATS:{
             printf("The case  ofp_port_stats_request \n" );
             type=Port_stats_Req_handle(sockfd,buffer,buf_len,&flow_length);
                 // printf("---Port_stats_Req_handle->header.length=%d---\n\n",flow_length);
             /*send(sockfd,mulReq,flow_length,0);
             if(type==ONU_INPUT)
               free(mulReq); */                                                
             break;
          }   
     case OFPMP_QUEUE:{
             printf("The case ofp_queue_stats_request \n" ); 
             queueReq = (struct ofp_queue_stats_request *)mulReq->body;
             if((htonl(queueReq->port_no) == OFPP_ANY))
                  send(sockfd,buffer,buf_len,0);
             else{
                  portno = Req_port_convert(htonl(queueReq->port_no));
                  queueReq->port_no = htonl(portno);
                  send(sockfd,mulReq,buf_len,0);
                 }//else              
             break;
          }          
     default:{
              printf("The case default multipart_request is sended \n" );
              send(sockfd,buffer,buf_len,0);
              break;
            }		                       
   }//switch
   	
}//multipart_request_handle

int Reply_port_convert(int port_no)
{
    printf("The Reply_port_convert from port_no: %d\n",port_no );
    int port;
    if( port_no >OF_OLT_CONNT_NUM )
        port = of_virtual_port(port_no);
    else{
   	     port = port_no;
    }
        
        
    printf("The Reply_port_no convert to : %d\n",port);      
        return  port;      
}//Reply_port_convert

void multipart_reply_handle(char* buffer, int buf_len,int sockfd)
{
    printf("------------------Staring handle 'multipart_reply' message from switch-------------\n\n");
    struct ofp_multipart_reply *mulReply;
    mulReply= (struct ofp_multipart_reply *)buffer;
    struct ofp_desc  *descReply;
    struct ofp_flow_stats *flowReply;
    struct ofp_aggregate_stats_reply *aggrReply;
    struct ofp_port_stats *portReply;
    struct ofp_queue_stats *queueReply;
    struct ofp_port *portDescReply;

    int portno,flow_length;
    cntlfd = sockfd ;
   switch(htons(mulReply->type)){
     case OFPMP_DESC:{
             printf("The reply body is struct ofp_desc .\n" );
             descReply = (struct ofp_desc *)mulReply->body;
   		     strcpy(descReply->mfr_desc, "CCU");
             strcpy(descReply->dp_desc, "SDN Proxy Switch");
             send(sockfd,buffer,buf_len,0);         
             break;
          }                                
     case OFPMP_FLOW:{
             printf("The reply body is an array of struct ofp_flow_stats.\n" );
             Flow_stats_Reply(buffer,buf_len,sockfd);     
             break;
          }
     case OFPMP_AGGREGATE:{
             printf("The reply body is struct ofp_aggregate_stats_reply. \n" );
             Aggre_stats_Reply(&mulReply,&flow_length);
             send(sockfd,mulReply,flow_length,0);                
             break;
          }      
     case OFPMP_PORT_STATS:{
             printf("The reply body is an array of struct ofp_port_stats \n" );
             portReply = (struct ofp_port_stats *)mulReply->body;
             portno = Reply_port_convert(htonl(portReply->port_no));
	     printf("The portno=%d.\n",portno);
             portReply->port_no = htonl(portno);
             send(sockfd,mulReply,buf_len,0);                                                 
             break;
          }   
     case OFPMP_QUEUE:{
             printf("The reply body is an array of struct ofp_queue_stats \n" );
             queueReply = (struct ofp_queue_stats *)mulReply->body;
             portno = Reply_port_convert(htonl(queueReply->port_no));
             queueReply->port_no = htonl(portno);
             send(sockfd,mulReply,buf_len,0);               
             break;
          } 
     case OFPMP_PORT_DESC:{
             printf("The reply body is an array of struct ofp_port. \n" );
             portDescReply = (struct ofp_port *)mulReply->body;
             portno = Reply_port_convert(htonl(portDescReply->port_no));
             portDescReply->port_no = htonl(portno);
             send(sockfd,mulReply,buf_len,0);               
             break;
          }               
     default:{
              printf("The reply body is an other \n" );
              send(sockfd,buffer,buf_len,0);
              break;
            }		                       
   }//switch
   	
}//multipart_reply_handle
