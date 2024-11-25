
==== mode: s option: none ===
 i 
---
 1
 2
 3
(3 rows)

FE=> Parse(stmt="", query="SELECT * FROM t1")
FE=> Bind(stmt="", portal="")
FE=> Execute(portal="")
FE=> Sync
<= BE ParseComplete
<= BE BindComplete
<= BE DataRow
<= BE DataRow
<= BE DataRow
<= BE CommandComplete(SELECT 3)
<= BE ReadyForQuery(I)
FE=> Terminate
==== mode: s option: terse ===
LOG:  ReadyForQuery message from backend 0
LOG:  ReadyForQuery message from backend 1
LOG:  RowDescription message from backend 1
LOG:  DataRow message from backend 1
LOG:  last DataRow message from backend 1 repeated 2 times
LOG:  CommandComplete message from backend 1
LOG:  ReadyForQuery message from backend 1
 i 
---
 1
 2
 3
(3 rows)

LOG:  ReadyForQuery message from backend 0
LOG:  ReadyForQuery message from backend 1
FE=> Parse(stmt="", query="SELECT * FROM t1")
FE=> Bind(stmt="", portal="")
FE=> Execute(portal="")
FE=> Sync
<= BE NoticeResponse(S LOG C XX000 M ParseComplete message from backend 1 
<= BE ParseComplete
<= BE NoticeResponse(S LOG C XX000 M BindComplete message from backend 1 
<= BE BindComplete
<= BE NoticeResponse(S LOG C XX000 M DataRow message from backend 1 
<= BE DataRow
<= BE DataRow
<= BE DataRow
<= BE NoticeResponse(S LOG C XX000 M last DataRow message from backend 1 repeated 2 times 
<= BE NoticeResponse(S LOG C XX000 M CommandComplete message from backend 1 
<= BE CommandComplete(SELECT 3)
<= BE NoticeResponse(S LOG C XX000 M ReadyForQuery message from backend 1 
<= BE ReadyForQuery(I)
FE=> Terminate
==== mode: s option: verbose ===
LOG:  ReadyForQuery message from backend 0
LOG:  ReadyForQuery message from backend 1
LOG:  RowDescription message from backend 1
LOG:  DataRow message from backend 1
LOG:  DataRow message from backend 1
LOG:  DataRow message from backend 1
LOG:  CommandComplete message from backend 1
LOG:  ReadyForQuery message from backend 1
 i 
---
 1
 2
 3
(3 rows)

LOG:  ReadyForQuery message from backend 0
LOG:  ReadyForQuery message from backend 1
FE=> Parse(stmt="", query="SELECT * FROM t1")
FE=> Bind(stmt="", portal="")
FE=> Execute(portal="")
FE=> Sync
<= BE NoticeResponse(S LOG C XX000 M ParseComplete message from backend 1 
<= BE ParseComplete
<= BE NoticeResponse(S LOG C XX000 M BindComplete message from backend 1 
<= BE BindComplete
<= BE NoticeResponse(S LOG C XX000 M DataRow message from backend 1 
<= BE DataRow
<= BE NoticeResponse(S LOG C XX000 M DataRow message from backend 1 
<= BE DataRow
<= BE NoticeResponse(S LOG C XX000 M DataRow message from backend 1 
<= BE DataRow
<= BE NoticeResponse(S LOG C XX000 M CommandComplete message from backend 1 
<= BE CommandComplete(SELECT 3)
<= BE NoticeResponse(S LOG C XX000 M ReadyForQuery message from backend 0 
<= BE NoticeResponse(S LOG C XX000 M ReadyForQuery message from backend 1 
<= BE ReadyForQuery(I)
FE=> Terminate
