#ifndef SCHEDULER_DEFS_H
#define SCHEDULER_DEFS_H

#define SCHEDULE_SUCCESS -1
#define SCHEDULE_HAS_LOOPED -2
#define SCHEDULE_PE_FREE_IS_GONE -3
#define SCHEDULER_INSUFFICIENT_PE_IN -4
#define SCHEDULER_INSUFFICIENT_PE_OUT -5
#define SCHEDULER_INSUFFICIENT_PE -6

const std::string scheduler_error_message[6]={
        "success",
        "schedule has looped",
        "PE free is gone",
        "insufficient input PE",
        "insufficient output PE",
        "insufficient PE"
};

#endif //SCHEDULER_DEFS_H
