#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <limits.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<netinet/in.h>
#include<pthread.h>
#include<string.h>
#include<signal.h>

#include "aws_iot_log.h"
#include "aws_iot_version.h"
#include "aws_iot_mqtt_interface.h"
#include "aws_iot_shadow_interface.h"
#include "aws_iot_config.h"


char certDirectory[PATH_MAX + 1] = "certs";
char HostAddress[255] = AWS_IOT_MQTT_HOST;
uint32_t port = AWS_IOT_MQTT_PORT;
MQTTClient_t mqttClient;

/*
 * @note The delta message is always sent on the "state" key in the json
 * @note Any time messages are bigger than AWS_IOT_MQTT_RX_BUF_LEN the underlying MQTT library will ignore it. The maximum size of the message that can be received is limited to the AWS_IOT_MQTT_RX_BUF_LEN
 */
char lastCommand[SHADOW_MAX_SIZE_OF_RX_BUFFER];

// Shadow Callback for receiving the delta
void DeltaCallback(const char *pJsonValueBuffer, uint32_t valueLength, jsonStruct_t *pJsonStruct_t);

void UpdateStatusCallback(const char *pThingName, ShadowActions_t action, Shadow_Ack_Status_t status,
		const char *pReceivedJsonDocument, void *pContextData);

/**
 * @brief This function builds a full Shadow expected JSON document by putting the data in the reported section
 *
 * @param pJsonDocument Buffer to be filled up with the JSON data
 * @param maxSizeOfJsonDocument maximum size of the buffer that could be used to fill
 * @param pReceivedDeltaData This is the data that will be embedded in the reported section of the JSON document
 * @param lengthDelta Length of the data
 */
bool buildJSONForReported(char *pJsonDocument, size_t maxSizeOfJsonDocument, char data[]) {
	int32_t ret;

	if (pJsonDocument == NULL) {
		return false;
	}

	char tempClientTokenBuffer[MAX_SIZE_CLIENT_TOKEN_CLIENT_SEQUENCE];

	if(aws_iot_fill_with_client_token(tempClientTokenBuffer, MAX_SIZE_CLIENT_TOKEN_CLIENT_SEQUENCE) != NONE_ERROR){
		return false;
	}

	ret = sprintf(pJsonDocument, "{\"state\":{\"reported\":%s,\"desired\":%s}, \"clientToken\":\"%s\"}",data,data, tempClientTokenBuffer);

	if (ret >= maxSizeOfJsonDocument || ret < 0) {
		return false;
	}

	return true;
}

void clearDelta()
{
	updateShadowStat("","","");
}

void updateShadowStat(char lastCmd[],char lastReply[],char lastCmdCache[])
{
	IoT_Error_t rc = NONE_ERROR;
	char tmp[100];
	
	sprintf(tmp,"{\"lastCommand\":\"%s\",\"lastReply\":\"%s#%s\"}",lastCmd,lastCmdCache,lastReply);
	
	if (buildJSONForReported(lastCommand, SHADOW_MAX_SIZE_OF_RX_BUFFER, tmp)) {
		rc = aws_iot_shadow_update(&mqttClient, AWS_IOT_MY_THING_NAME, lastCommand, UpdateStatusCallback, NULL, 2, true);
		if (NONE_ERROR != rc) {
			ERROR("An error occurred while updating shadow - %d", rc);
		}
	}
}

void incaseOfSignal()
{
	exit(0);
}

int main(int argc, char** argv) {
	IoT_Error_t rc = NONE_ERROR;
	int32_t i = 0;

	char rootCA[PATH_MAX + 1];
	char clientCRT[PATH_MAX + 1];
	char clientKey[PATH_MAX + 1];
	char CurrentWD[PATH_MAX + 1];
	char cafileName[] = AWS_IOT_ROOT_CA_FILENAME;
	char clientCRTName[] = AWS_IOT_CERTIFICATE_FILENAME;
	char clientKeyName[] = AWS_IOT_PRIVATE_KEY_FILENAME;

	signal(SIGALRM, incaseOfSignal);
	
	getcwd(CurrentWD, sizeof(CurrentWD));
	sprintf(rootCA, "/root/certs/%s",cafileName);
	sprintf(clientCRT, "/root/certs/%s", clientCRTName);
	sprintf(clientKey, "/root/certs/%s", clientKeyName);


	// initialize the mqtt client
	aws_iot_mqtt_init(&mqttClient);

	ShadowParameters_t sp = ShadowParametersDefault;
	sp.pMyThingName = AWS_IOT_MY_THING_NAME;
	sp.pMqttClientId = AWS_IOT_MQTT_CLIENT_ID;
	sp.pHost = AWS_IOT_MQTT_HOST;
	sp.port = AWS_IOT_MQTT_PORT;
	sp.pClientCRT = clientCRT;
	sp.pClientKey = clientKey;
	sp.pRootCA = rootCA;

	INFO("Shadow Init");
	rc = aws_iot_shadow_init(&mqttClient);
	if (NONE_ERROR != rc) {
		ERROR("Shadow Connection Error");
		return rc;
	}

	INFO("Shadow Connect");
	rc = aws_iot_shadow_connect(&mqttClient, &sp);
	if (NONE_ERROR != rc) {
		ERROR("Shadow Connection Error");
		return rc;
	}

	/*
	 * Enable Auto Reconnect functionality. Minimum and Maximum time of Exponential backoff are set in aws_iot_config.h
	 *  #AWS_IOT_MQTT_MIN_RECONNECT_WAIT_INTERVAL
	 *  #AWS_IOT_MQTT_MAX_RECONNECT_WAIT_INTERVAL
	 */
	rc = mqttClient.setAutoReconnectStatus(true);
	if(NONE_ERROR != rc){
		ERROR("Unable to set Auto Reconnect to true - %d", rc);
		return rc;
	}

	jsonStruct_t command;
	command.pData = lastCommand;
	command.pKey = "lastCommand";
	command.type = SHADOW_JSON_STRING;
	command.cb = DeltaCallback;

	/*
	 * Register the jsonStruct object
	 */
	rc = aws_iot_shadow_register_delta(&mqttClient, &command);

	clearDelta();
	
	// Now wait in the loop to receive any message sent from the console
	while (NETWORK_ATTEMPTING_RECONNECT == rc || RECONNECT_SUCCESSFUL == rc || NONE_ERROR == rc || rc == -28) {
		/*
		 * Lets check for the incoming messages for 200 ms.
		 */
		rc = aws_iot_shadow_yield(&mqttClient, 200);
		
		if (NETWORK_ATTEMPTING_RECONNECT == rc) {
			sleep(1);
			// If the client is attempting to reconnect we will skip the rest of the loop.
			continue;
		}

		// sleep for some time in seconds
		sleep(1);
	}

	if (NONE_ERROR != rc) {
		ERROR("An error occurred in the loop %d", rc);
	}

	INFO("Disconnecting");
	rc = aws_iot_shadow_disconnect(&mqttClient);

	if (NONE_ERROR != rc) {
		ERROR("Disconnect error %d", rc);
	}

	return rc;
}

void askServer(char command[])
{
    int internal_client_socketid = 0;
    char reply[20];
    
    // set alarm of 5 seconds
    alarm (5);
    
    // create socket to internal server
    struct sockaddr_in serv_addr;

    if((internal_client_socketid = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        exit(0);
    }

    memset(&serv_addr, '0', sizeof(serv_addr));

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(5000);

    if(inet_pton(AF_INET,"192.168.0.1",&serv_addr.sin_addr)<=0)
    {
        exit(0);
    }

    // connect to internal server
    if( connect(internal_client_socketid, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        exit(0);
    }
    
    // pass it on to internal server
    write(internal_client_socketid,command,strlen(command)+1);

    // get reply from internal server
    read(internal_client_socketid,reply,4096);
    
    close(internal_client_socketid);
    
    // disable alarm
    alarm (0);
    updateShadowStat("",reply,command);
    
}

void DeltaCallback(const char *pJsonValueBuffer, uint32_t valueLength, jsonStruct_t *pJsonStruct_t) {

	char command[20];
	DEBUG("Received Delta message %.*s", valueLength, pJsonValueBuffer);
	snprintf(command,20,"%.*s",valueLength, pJsonValueBuffer);
	askServer(command);
}

void UpdateStatusCallback(const char *pThingName, ShadowActions_t action, Shadow_Ack_Status_t status,
		const char *pReceivedJsonDocument, void *pContextData) {

	if (status == SHADOW_ACK_TIMEOUT) {
		INFO("Update Timeout--");
	} else if (status == SHADOW_ACK_REJECTED) {
		INFO("Update RejectedXX");
	} else if (status == SHADOW_ACK_ACCEPTED) {
		INFO("Update Accepted !!");
	}
}
