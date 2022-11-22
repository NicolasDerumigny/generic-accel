set0m mean
noop
noop
noop

accsumcm mean, data
noop
noop
noop

noop
noop
noop
divvs mean, mean, float_n

set0m stddev
noop
noop
noop

subcmv tmp1, data, mean
noop
noop
noop

pmulm tmp2, tmp1, tmp1
noop
noop
noop

accsumcm stddev, tmp2
noop
noop
noop

noop
noop
noop
divms stddev, stddev, float_n

noop
noop
noop
sqrtv stddev, stddev

cutminv stddev, stddev
noop
noop
noop

subcmv data, data, mean
noop
noop
noop

mulsv stddev, sqrtfloat_n, stddev
noop
noop
noop

noop
noop
noop
divcmv data, data, stddev

trm tmp3, data
noop
noop
noop

mulmm corr, tmp3, data
noop
noop
noop

setd1 corr, corr
noop
noop
noop
