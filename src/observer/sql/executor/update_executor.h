#pragma once

#include "common/sys/rc.h"

class SQLStageEvent;

/**
 * @brief UPDATE 语句执行器
 * @ingroup Executor
 */
class UpdateExecutor
{
public:
  UpdateExecutor()          = default;
  virtual ~UpdateExecutor() = default;

  RC execute(SQLStageEvent *sql_event);
};