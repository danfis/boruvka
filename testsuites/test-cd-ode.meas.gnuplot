plot "test-cd-ode.meas" u 1:2 w l title "update dirty", \
     "test-cd-ode.meas" u 1:($2 + $3) w l title "radix[0]", \
     "test-cd-ode.meas" u 1:($2 + $3 + $4) w l title "radix[1]", \
     "test-cd-ode.meas" u 1:($2 + $3 + $4 + $5) w l title "radix[2]", \
     "test-cd-ode.meas" u 1:($2 + $3 + $4 + $5 + $6) w l title "remove pairs", \
     "test-cd-ode.meas" u 1:($2 + $3 + $4 + $5 + $6 + $7) w l title "find pairs", \
     "test-cd-ode.meas" u 1:8 w l title "separate";

pause -1
