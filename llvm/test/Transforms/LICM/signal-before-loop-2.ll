; REQUIRES: enable_build_for_common
; RUN:opt -disable-move-store-ins-outside-of-loop=true -S < %s | FileCheck %s 

@Run_Index = external global i64

declare ptr @signal(ptr)

define void @report() {
entry:
  %0 = load i64, ptr @Run_Index, align 8
  unreachable
}

define i32 @main() {
if.end:
  %call.i4 = call ptr @signal(ptr @report)
  br label %for.cond

; CHECK-LABEL: for.cond
; CHECK: store
for.cond:
  %0 = load i64, ptr @Run_Index, align 8
  store i64 %0, ptr @Run_Index, align 8
  br label %for.cond
}
