#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "openflow_1_3.h"
#include "proxy_table.h"

enum flow_direct_type{
	U_ONU_TO_OFSW,
	D_OFSW_TO_ONU,
	UL_OFSW_TO_OFSW,
	DL_ONU_TO_ONU,
	NOT_OUTPUT,
	ONU_INPUT,
	OFSW_INPUT,
	OTHER,
	FLOW_D_ERROR,
	OTHER_ERROR
};
int match_inport_covert(int *inport,int *use_vlan,struct virtual_onu_olt_port *qurey);
int read_match(void *readP,enum oxm_ofb_match_fields field);

void modify_match(void *modifyP,enum oxm_ofb_match_fields field,int value);
enum flow_direct_type Flow_direct(int in_port,int out_port);
void Flow_action_output(struct ofp_instruction_actions  **instruction_actions,struct ofp_action_header **action,int value);
void Flow_action_set_field(struct ofp_instruction_actions  **instruction_actions,struct ofp_action_header **action,int value);
void Flow_action_push(struct ofp_instruction_actions  **instruction_actions,struct ofp_action_header **action,enum ofp_action_type type, uint16_t ethertype);
void Flow_action_header(struct ofp_instruction_actions  **instruction_actions,struct ofp_action_header **action,enum ofp_action_type type);
void  Flow_instruction_goto_table(struct ofp_instruction_goto_table  **instruction_goto_table,struct ofp_action_header **action,uint8_t table_id);
void Flow_instruction_actions(void **matchP,struct ofp_instruction_actions  **instruction_actions,enum ofp_instruction_type type);
void Flow_Match(void *matchP,uint32_t oxm_type,int value);
int read_action(void *read_aP,enum ofp_action_type type);
int Flow_offset(uint8_t**oxm_tlv,uint8_t*pOxm_tlv,int length);
void Actions_type_address(struct ofp_action_header **action_header,uint16_t len,enum ofp_action_type type);

uint8_t* Least_oxm_address(void *p,int length);
uint8_t Read_payload(void *p,int payload_len);
void Oxm_match_printf(uint8_t*pOxm_tlv);
