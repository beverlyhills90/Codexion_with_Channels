#include "../codexion.h"
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>

typedef struct s_checkResult {
    t_checkStatus stauts;
    long long time;
    unsigned int burnedCoderId;
} t_checkResult ;


void print_log(t_msg *msg, t_worldData *worldData)
{
    long long logTime;
    char *logs[4];

    logs[0] = " has taken a dongle";
    logs[1] = " is compiling";
    logs[2] = " is debugging";
    logs[3] = " is refactoring";
    if(safeWorldStateCheck(worldData) == STOP)
        return;
    logTime = msg->timestomp - worldData->timeOfStart;
    printf("%llu %i%s\n",logTime, msg->coderId, logs[msg->type]);
}

void set_complie(t_worldData *wordData, t_msg *msg)
{
    wordData->lastComplieTimeArr[msg->coderId-1] = msg->timestomp - wordData->timeOfStart;
    wordData->compilationsDone[msg->coderId-1]++;
}

t_checkResult checkBurnOut(t_worldData *worldData,long long timeOfStart)
{
    unsigned long long min_time;
    size_t i;
    size_t done;
    t_checkResult result;

    i = 0;
    done = 0;
    result.stauts = RS_BURNEDOUT;
    min_time = worldData->lastComplieTimeArr[0];
    while (i < worldData->args->number_of_coders)
    {
        if (worldData->lastComplieTimeArr[i] < min_time)
            min_time = worldData->lastComplieTimeArr[i];
        if (worldData->lastComplieTimeArr[i] + worldData->args->time_to_burnout <= get_ms() - timeOfStart)
        {
            worldStop(worldData);
            result.time = worldData->lastComplieTimeArr[i] + worldData->args->time_to_burnout;
            result.burnedCoderId = i + 1;
            return result;
        }
        if (worldData->args->number_of_compiles_required == worldData->compilationsDone[i])
            done++;
        i++;
    }
    if (done == i)
        result.stauts = RS_DONE;
    else
        result.stauts = RS_OK;
    result.time = get_ms() - min_time;
    return result;
}

void *monitor(void *arg)
{
    t_worldData *worldData;
    t_chan_result res;
    t_msg *msg;
    unsigned int burnOutId;
    long long timeOfBurnOut;
    t_checkResult check_res;

    worldData = (t_worldData *)arg;
    while (1)
    {
        check_res = checkBurnOut(worldData,worldData->timeOfStart);
        if (check_res.stauts == RS_BURNEDOUT)
        {
            printf("STOP");
            printf("%llu %u burned out\n",check_res.time,check_res.burnedCoderId);
            return (NULL);
        }
        else if (check_res.stauts == RS_DONE) 
            return (worldStop(worldData), NULL);
        res = mpsc_recv_until(worldData->log_rcv,check_res.time + worldData->args->time_to_burnout);
        msg = res.data;
        if (res.status == CH_CLOSED)
            break;
        if (res.status == CH_TIMEOUT)
            continue;
        if (msg->type != MSG_COMPILE_DONE)
            print_log(msg, worldData);
        if (msg->type == MSG_COMPILING)
            set_complie(worldData,msg);
        free(msg);
    }
    return (NULL);
}