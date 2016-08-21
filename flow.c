#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include"channel_communication.h"
#include "flow.h"
#include "oxm_match.h"
//#include "proxy_table.h"
//extern struct virtual_onu_olt_port *pOnutab;


int match_inport_covert(int *inport,int *use_vlan,struct virtual_onu_olt_port *qurey)//covert from virtual_port
{
    printf("---------match_inport_covert ------------\n\n" );
    	int in_port,new_in_port;
	int direct;
	int a;
	struct virtual_onu_olt_port *p;
	in_port  = *inport;
	if(in_port>ONU_PORT_NUM){
		new_in_port=of_init_port(in_port);
		*use_vlan = -1;
		printf("in_port:%d,new_in_port:%d\n\n",in_port,new_in_port);
	    direct = OFSW_INPUT;
	}	
	if(in_port<=ONU_PORT_NUM){/*The in_port from ONU*/
		p = qurey;
		for(a=0;a<ONU_PORT_NUM;a++,p++){
			if(p->vir_port==in_port){
				printf("in_port:%d,olt_of_port:%d,used_vlan:%d\n\n",in_port,p->olt_of_port,p->used_vlan);
				*use_vlan = p->used_vlan;
				new_in_port=p->olt_of_port;
				direct = ONU_INPUT;
				break;
			}
		}
	} 
	*inport = new_in_port;
	return direct;
}//match_inport_covert

int read_match(void *readP,enum oxm_ofb_match_fields field)
{
  //printf("--------- read_match------------\n" );
  int value,match_len;
  uint8_t *pOxm_tlv;
  struct ofp_match *m;	
	m = (struct ofp_match *)readP;
	pOxm_tlv = &m->oxm_fields;////////////////////////////////////////?
	match_len=htons(m->length)-4;//The size of type & len is 4 byte 
	//printf("--------- match_len=%d------------\n",match_len);
	while(OXM_FIELD(UNPACK_OXM_TLV(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3)))!=field && match_len>0)
	{
		match_len-=(OXM_LENGTH( UNPACK_OXM_TLV(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3)))+4);	
		pOxm_tlv+=(OXM_LENGTH(UNPACK_OXM_TLV(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3)))+4);
	}

	if(OXM_FIELD(UNPACK_OXM_TLV(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3)))==field)
		value = Read_payload(pOxm_tlv+4,(OXM_LENGTH(UNPACK_OXM_TLV(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3)))));
	else
		value =-1;
  return value;	
}//read_match


void modify_match(void *modifyP,enum oxm_ofb_match_fields field,int value)
{
  int match_len;
  uint32_t *w_oxm_tlv,oxm_type;
  uint16_t *w_2byte_oxm_tlv; 
  uint8_t *pOxm_tlv;
  struct ofp_match *m;	
	
	m = (struct ofp_match *)modifyP;
	pOxm_tlv = m->oxm_fields;
	match_len=htons(m->length)-4;//The size of type & len is 4 byte 
	while(OXM_FIELD(UNPACK_OXM_TLV(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3)))!=field && match_len>0)
	{
		match_len-=(OXM_LENGTH( UNPACK_OXM_TLV(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3)))+4);	
		pOxm_tlv+=(OXM_LENGTH(UNPACK_OXM_TLV(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3)))+4);
    
	}

	  switch(field) {
		case OFPXMT_OFB_IN_PORT:{
				oxm_type = OXM_OF_IN_PORT;
				printf("case OFPXMT_OFB_IN_PORT\n" );
			 break; 
		}
		case OFPXMT_OFB_VLAN_VID:{
 				oxm_type = OXM_OF_VLAN_VID;
				printf("case  OFPXMT_OFB_VLAN_VID\n" );
			 break; 
		}
		default:{
			printf("The oxm_type is not exist\n" );
			break;
		}
	  }//switch
     
	if(OXM_FIELD(UNPACK_OXM_TLV(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3)))==field)
	{
          
	  switch(oxm_type) {
		case OXM_OF_IN_PORT:{
			 w_oxm_tlv = (uint32_t*)pOxm_tlv;
			*w_oxm_tlv =  ntohl(oxm_type);
			*(w_oxm_tlv+1)=ntohl(value);
			printf("value:%d\n\n",value);
			 break; 
		}
		case OXM_OF_VLAN_VID:{
 			 w_oxm_tlv = (uint32_t*)pOxm_tlv;
 			*w_oxm_tlv =  ntohl(oxm_type);
  			 w_2byte_oxm_tlv= (uint16_t *)(w_oxm_tlv+1);/////problem
 			*(w_2byte_oxm_tlv)=ntohs(value);
			printf("value:%d\n\n",value);
			 break; 
		}
		default:{
			printf("The oxm_type is not exist\n" );
			break;
		}
	  }//switch
	}//if  
		
}// modify_match

enum flow_direct_type Flow_direct(int in_port,int out_port)
{
  int type;		
	
     if(out_port==OFPP_ANY)//out_port==OFPP_ANY
       {
          printf("The out_port==OFPP_ANY\n" );
          type = NOT_OUTPUT;                                                                         
       } 
     else if(0<in_port && in_port<=ONU_PORT_NUM && out_port>ONU_PORT_NUM)//ONU_TO_OFSW
       {
          printf("The flow direction is 'U_ONU_TO_OFSW'\n" );
          type =U_ONU_TO_OFSW;
	   }
     else if(in_port>ONU_PORT_NUM && out_port<=ONU_PORT_NUM)//OFSW_TO_ONU
       {
          printf("The flow direction is 'D_OFSW_TO_ONU'\n" );
          type = D_OFSW_TO_ONU;
	   }
     else if(in_port>ONU_PORT_NUM && out_port>ONU_PORT_NUM)//OFSW_TO_OFSW;
       {
          printf("The flow direction is 'UL_OFSW_TO_OFSW'\n" );
          type = UL_OFSW_TO_OFSW;
	   }
     else if(0<in_port && in_port<=ONU_PORT_NUM && out_port<=ONU_PORT_NUM)//ONU_TO_ONU
       {
          printf("The flow direction is 'DL_ONU_TO_ONU'\n" );
          type= DL_ONU_TO_ONU;
       }
	else{
	      printf("The refer is not correct\n" );
		  type=FLOW_D_ERROR;
       }
	return type;
}

void Flow_action_output(struct ofp_instruction_actions  **instruction_actions,struct ofp_action_header **action,int value)
{

  struct ofp_instruction_actions  *instruction; 
  struct ofp_action_output *output;


  instruction =* instruction_actions;

  if(*action==NULL)
  	output = (struct ofp_action_output*)instruction->actions;
  else
	output =*(struct ofp_action_output**)action;///////////////////////////////////problem	

  output->type = htons(OFPAT_OUTPUT);     
  output->len = htons(sizeof(struct ofp_action_output));       
  output->port =htonl(value);
  output->max_len =htons(0);

  instruction->len  = instruction->len+output->len;


}//Flow_action_output

void Flow_action_set_field(struct ofp_instruction_actions  **instruction_actions,struct ofp_action_header **action,int value)
{
  struct ofp_instruction_actions  *instruction;
  struct ofp_action_set_field *set_field;
  uint32_t *w_oxm_action_field;
  uint16_t *w_2byte_oxm_tlv;
  uint8_t  oxm_len,*pOxm_tlv,*least_oxm_address;

  instruction =* instruction_actions;

  if(*action==NULL)
  	set_field = instruction->actions;
  else
  	set_field =*action;

  set_field->type = htons(OFPAT_SET_FIELD); 
  w_oxm_action_field = pOxm_tlv = &set_field->field;
  *w_oxm_action_field =  ntohl(OXM_OF_VLAN_VID);
  w_2byte_oxm_tlv= (w_oxm_action_field+1);//w_oxm_action_field+1 =oxm_tlv_header memory 4byte
  *(w_2byte_oxm_tlv)=ntohs(value); 
  least_oxm_address=w_2byte_oxm_tlv+1;
  oxm_len = OXM_LENGTH(UNPACK_OXM_TLV(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3)));
  set_field->len = htons(sizeof(struct ofp_action_set_field))+htons(OFP_ACTION_SET_FIELD_OXM_PADDING(oxm_len+4))+htons(oxm_len);//oxm_tlv_field 4byte
  
  instruction->len  = instruction->len+set_field->len;
/*
printf("Proxy_set_field->len%d\n\n",htons(set_field->len));
printf("Proxy_instruction->len:%d\n\n",htons(instruction->len));
printf("Proxy_action_header_length:%d\n\n",htons(flow_mod->header.length));*/

  *action= least_oxm_address+OFP_ACTION_SET_FIELD_OXM_PADDING(oxm_len+4);//next pointer address
}//Flow_action_set_field

void Flow_action_push(struct ofp_instruction_actions  **instruction_actions,struct ofp_action_header **action,enum ofp_action_type type, uint16_t ethertype)
{

  struct ofp_action_push *push;
  struct ofp_instruction_actions  *instruction;

  instruction =* instruction_actions;
  
  if(*action==NULL)
  	push = (struct ofp_action_push *)instruction->actions;
  else
	push =*(struct ofp_action_push **)action;

  push->type = htons(type);     
  push->len = htons(sizeof(struct ofp_action_push));       
  push->ethertype = htons(ethertype); 
  instruction->len  = instruction->len+push->len;
/*
printf("Flow_instruction_push->len:%d\n\n",htons(instruction->len));
*/
  *action= (struct ofp_action_header *)(push+1) ;//next pointer address
}//Flow_action_push

void Flow_action_header(struct ofp_instruction_actions  **instruction_actions,struct ofp_action_header **action,enum ofp_action_type type)
{

  struct ofp_instruction_actions  *instruction;
  struct ofp_action_header *action_header;
  
  instruction =* instruction_actions;

  if(*action==NULL)
  	action_header = (struct ofp_action_header*)instruction->actions;
  else
	action_header =*action;	

  action_header->type = htons(type);     
  action_header->len = htons(sizeof(struct ofp_action_header));       

  instruction->len  = instruction->len+action_header->len;

/*
printf("Flow_action_header->len:%d\n\n",htons(action_header->len));
printf("Flow_instruction->len:%d\n\n",htons(instruction->len));
*/

  *action= action_header+1;//next pointer	
}//Flow_action_header

void  Flow_instruction_goto_table(struct ofp_instruction_goto_table  **instruction_goto_table,struct ofp_action_header **action,uint8_t table_id)
{
  struct ofp_instruction_goto_table *goto_table;

  goto_table = *(struct ofp_instruction_goto_table **)action;		
  goto_table->type = htons(OFPIT_GOTO_TABLE);
  goto_table->len  = htons(sizeof(struct ofp_instruction_goto_table));
  goto_table->table_id = table_id;

  *instruction_goto_table = goto_table;
}// Flow_instruction_goto_table

void Flow_instruction_actions(void **matchP,struct ofp_instruction_actions  **instruction_actions,enum ofp_instruction_type type)
{
  uint8_t *pOxm_tlv;
  struct ofp_instruction_actions *instruction;
  struct ofp_match *m;
  
  m = *(struct ofp_match **)matchP; 
  Flow_offset(&pOxm_tlv, m->oxm_fields,htons(m->length));
  instruction =(struct ofp_instruction_actions*)(pOxm_tlv);
  instruction->type = htons(type);
  instruction->len  = htons(sizeof(struct ofp_instruction_actions));
	
}// Flow_instruction_actions

void Flow_Match(void *matchP,uint32_t oxm_type,int value)
{
  printf("---------The Flow_Match is created-----\n\n" );   	
  uint8_t *address;
  uint32_t *w_oxm_tlv;
  uint16_t *w_2byte_oxm_tlv; 
  uint16_t  match_len;
  struct ofp_match *m;
  //void **p;
  
  m = (struct ofp_match *)matchP;
  //pOxm_tlv = m->oxm_fields;
  
  /*Initial set
  m->type	= htons(OFPMT_OXM);
  m->length = htons(sizeof(struct ofp_match));*/
  printf("m->length:%d\n\n",htons(m->length));
  if(m->length==htons(sizeof(struct ofp_match))){//If first match
 	 m->length = m->length-htons(4); //match.oxm_fields is 4 byte 
	 address = &m->oxm_fields;
  }else{
	 address = Least_oxm_address(&m->oxm_fields,htons(m->length));
  }		
  switch(oxm_type) {
		case OXM_OF_IN_PORT:{
			 w_oxm_tlv = (uint32_t*)address;
			*w_oxm_tlv =  ntohl(oxm_type);
			*(w_oxm_tlv+1)=ntohl(value);
			 match_len = htons(4+4); //match.oxm_fields is 4byte ,and value is 4byte
			 break; 
		}
		case OXM_OF_VLAN_VID:{
 			 w_oxm_tlv = (uint32_t*)address;
 			*w_oxm_tlv =  ntohl(oxm_type);
  			 w_2byte_oxm_tlv= (uint16_t*)(w_oxm_tlv+1);
 			*(w_2byte_oxm_tlv)=ntohs(value);
			 match_len = htons(4+2);//match.oxm_fields is 4byte ,and value is 2byte
			 break; 
		}
		default:{
			printf("The oxm_type is not exist\n" );
			break;
		}
  }//switch
  m->length  =  m->length + match_len; 
  printf("New_m->length:%d\n\n",htons(m->length));
}//Flow_Match


int read_action(void *read_aP,enum ofp_action_type type)
{
  int value;
  uint8_t *pOxm_tlv;
  char * cOxm_tlv;	
  struct ofp_instruction_actions* inst_actions;
  struct ofp_action_header *action;
  struct ofp_match *m;
  struct ofp_instruction * inst_check;
  
  	m = (struct ofp_match *)read_aP;
	pOxm_tlv = m->oxm_fields;
  	Flow_offset(&pOxm_tlv, m->oxm_fields,htons(m->length));
//Meter
	inst_check = (struct ofp_instruction *)(pOxm_tlv);
  	cOxm_tlv = (char*)pOxm_tlv;
  	if (htons(inst_check->type)!=OFPIT_APPLY_ACTIONS)
	    inst_actions =(struct ofp_instruction_actions*)(cOxm_tlv+htons(inst_check->len));	
  	else
	    inst_actions =(struct ofp_instruction_actions*)(pOxm_tlv);

	action =(struct ofp_action_header *)inst_actions->actions;
	Actions_type_address(&action,inst_actions->len,type);
	switch(type){
		case OFPAT_OUTPUT:{
				printf("Action header that is 'OFPAT_OUTPUT'\n\n" );
				struct ofp_action_output *p;
				p = (struct ofp_action_output *)action;
				value = htonl(p->port);
				break;
		}
		case OFPAT_PUSH_VLAN:{
				printf("Action header that is 'OFPAT_PUSH_VLAN'\n\n" );
				break;
		}
		case OFPAT_POP_VLAN:{
				printf("Action header that is 'OFPAT_POP_VLAN'\n\n" );
				break;
		}
		case OFPAT_SET_FIELD:{
				printf("Action header that is 'OFPAT_SET_FIELD'\n\n" );
				break;
		}
		default:{
			printf("Action header that is not exist!\n" );
			break;
		}	
	}//switch

  return value;		
}//read_action

int Flow_offset(uint8_t**oxm_tlv,uint8_t*pOxm_tlv,int length)
{
	 int oxm_tvl_num = 0,match_len=length-4;	
	 while(match_len>0){
		//Oxm_match_printf(pOxm_tlv);
		match_len-=(OXM_LENGTH( UNPACK_OXM_TLV(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3)))+4);
		pOxm_tlv+=(OXM_LENGTH( UNPACK_OXM_TLV(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3)))+4);
		oxm_tvl_num++;
		//printf("\noxm_tvl_num:%d\n\n",oxm_tvl_num);
	}
	*oxm_tlv=pOxm_tlv+ OFP_MATCH_OXM_PADDING(length);
	
	return oxm_tvl_num;
}

void Actions_type_address(struct ofp_action_header **action_header,uint16_t len,enum ofp_action_type type)
{
printf("----------------------actions_type_address -----------------------\n\n");
	struct ofp_action_header *action;
	action = *action_header;
	while(len>0&&(htons(action->type)!=type))
	{
	  len=len-(action->len);	
	  action=action+(htons(action->len)/8);
	}
	*action_header=action;	
}


uint8_t* Least_oxm_address(void *p,int length)
{
  uint8_t*pOxm_tlv;
  pOxm_tlv = (uint8_t*)p;
  int match_len=length-4;	

	 while(match_len>0){
		match_len-=(OXM_LENGTH( UNPACK_OXM_TLV(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3)))+4);
		pOxm_tlv+=(OXM_LENGTH( UNPACK_OXM_TLV(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3)))+4);
	}
	return pOxm_tlv;
}

uint8_t Read_payload(void *p,int payload_len)
{
	uint8_t value;
  	uint8_t*pOxm_tlv;
  	pOxm_tlv = (uint8_t*)p;
	switch(payload_len){
		case 8:{
			value = UNPACK_OXM_TLV_PAYLOAD_8_BYTE(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3),*(pOxm_tlv+4),*(pOxm_tlv+5),*(pOxm_tlv+6),*(pOxm_tlv+7));
			break;
		}
		case 6:{
			value = UNPACK_OXM_TLV_PAYLOAD_6_BYTE(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3),*(pOxm_tlv+4),*(pOxm_tlv+5));
			break;
		}
		case 4:{
			value = UNPACK_OXM_TLV_PAYLOAD_4_BYTE(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3));
			break;
		}
		case 2:{
			value = UNPACK_OXM_TLV_PAYLOAD_2_BYTE(*pOxm_tlv,*(pOxm_tlv+1));
			break;
		}
	}//switch
return value;
}

void Oxm_match_printf(uint8_t*pOxm_tlv)
{
	printf("The is printing to oxm_match...\n\n");
	printf("CLASS=0x%x,FIELD=%d,TYPE=%04x,HASMASK=%d,LENGTH=%d,VAULE=%x\n\n",\
	OXM_CLASS(UNPACK_OXM_TLV(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3))),\
	OXM_FIELD(UNPACK_OXM_TLV(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3))),\
	OXM_TYPE( UNPACK_OXM_TLV(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3))),\
	OXM_HASMASK( UNPACK_OXM_TLV(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3))),\
	OXM_LENGTH( UNPACK_OXM_TLV(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3))),\
	Read_payload(pOxm_tlv+4,(OXM_LENGTH(UNPACK_OXM_TLV(*pOxm_tlv,*(pOxm_tlv+1),*(pOxm_tlv+2),*(pOxm_tlv+3))))));
	
}


