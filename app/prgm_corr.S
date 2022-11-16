set0m mean
noop

accsumcm mean, data
noop

divvs mean, mean, float_n
noop

set0m stddev
noop

subcmv tmp1, data, mean
noop

pmulm tmp2, tmp1, tmp1
noop

accsumcm stddev, tmp2
noop

divms stddev, stddev, float_n
noop

sqrtv stddev, stddev
noop

cutminv stddev, stddev
noop

subcmv data, data, mean
noop

mulsv stddev, sqrtfloat_n, stddev
noop

divcmv data, data, stddev
noop

trm tmp3, data
noop

mulmm corr, tmp3, data
noop

setd1 corr, corr
noop
