## LLVM
### Build
```bash
cd build/
make
```

### Run
```
./calculator
```

### Test
```
canmi@xyy /o/u/p/c/c/build (main)> ./calculator 
```

```
Enter expression: ((3 + 5 * sin(pi / 4)) ^ 2 - log(sqrt(16)) * cos(atan(1))) / 3 + pi * e
; ModuleID = 'calc'
source_filename = "calc"

define double @main() {
entry:
  %sintmp = call double @sin(double 0x3FE921FB54442D18)
  %multmp = fmul double 5.000000e+00, %sintmp
  %addtmp = fadd double 3.000000e+00, %multmp
  %powtmp = call double @pow(double %addtmp, double 2.000000e+00)
  %sqrttmp = call double @sqrt(double 1.600000e+01)
  %logtmp = call double @log(double %sqrttmp)
  %atantmp = call double @atan(double 1.000000e+00)
  %costmp = call double @cos(double %atantmp)
  %multmp1 = fmul double %logtmp, %costmp
  %subtmp = fsub double %powtmp, %multmp1
  %divtmp = fdiv double %subtmp, 3.000000e+00
  %addtmp2 = fadd double %divtmp, 0x402114580B45D474
  ret double %addtmp2
}

declare double @sin(double)

declare double @pow(double, double)

declare double @sqrt(double)

declare double @log(double)

declare double @atan(double)

declare double @cos(double)
Result: 22.4507
```
