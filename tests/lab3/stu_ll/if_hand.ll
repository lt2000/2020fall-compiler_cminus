; ModuleID = 'if.c'
source_filename = "if.c"
target datalayout = "e-m:e-p270:32:32-p271:32:32-p272:64:64-i64:64-f80:128-n8:16:32:64-S128"
target triple = "x86_64-unknown-linux-gnu"


; Function Attrs: noinline nounwind optnone uwtable
define dso_local i32 @main() #0 {
  %1 = alloca i32, align 4
  %2 = alloca float, align 4                          ;%2是指针
  store i32 0, i32* %1, align 4
  store float 0x40163851E0000000, float* %2, align 4  ;5.555怎么存？？？ IEEE 754 double
  %3 = load float, float* %2
  %4 = fcmp ogt float %3, 1.000000                    ;条件判断
  br i1 %4, label %5, label %6

5:                                              
  store i32 233, i32* %1, align 4                     ;a>1
  br label %7                                         ;转到返回
             
6:                                                
  store i32 0, i32* %1, align 4                       ;a<=1
  br label %7                                         ;转到返回

7:                                               
  %8 = load i32, i32* %1, align 4                     ;ret分支
  ret i32 %8
}

attributes #0 = { noinline nounwind optnone uwtable "correctly-rounded-divide-sqrt-fp-math"="false" "disable-tail-calls"="false" "frame-pointer"="all" "less-precise-fpmad"="false" "min-legal-vector-width"="0" "no-infs-fp-math"="false" "no-jump-tables"="false" "no-nans-fp-math"="false" "no-signed-zeros-fp-math"="false" "no-trapping-math"="false" "stack-protector-buffer-size"="8" "target-cpu"="x86-64" "target-features"="+cx8,+fxsr,+mmx,+sse,+sse2,+x87" "unsafe-fp-math"="false" "use-soft-float"="false" }

!llvm.module.flags = !{!0}
!llvm.ident = !{!1}

!0 = !{i32 1, !"wchar_size", i32 4}
!1 = !{!"clang version 10.0.1 "}
