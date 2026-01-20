#include "sql/stmt/drop_table_stmt.h"
#include "storage/db/db.h"
#include "storage/table/table.h"
#include "common/log/log.h"
#include "event/session_event.h"
#include "event/sql_event.h"
#include "session/session.h"
#include "sql/executor/drop_table_executor.h"

RC DropTableExecutor::execute(SQLStageEvent *sql_event)
{
    Stmt         *stmt          = sql_event->stmt();
    SessionEvent *session_event = sql_event->session_event();
    Session      *session       = session_event->session();
    // 检查语句类型
    ASSERT(stmt->type() == StmtType::DROP_TABLE,
        "drop table executor can not run this command: %d",
        static_cast<int>(stmt->type()));

    DropTableStmt *drop_table_stmt = static_cast<DropTableStmt *>(stmt);
    const char    *table_name      = drop_table_stmt->table_name().c_str();

    RC rc = session->get_current_db()->drop_table(table_name);

    return rc;
    
}
