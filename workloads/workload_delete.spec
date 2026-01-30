# Delete-only workload
recordcount=100000
operationcount=100000
workload=com.yahoo.ycsb.workloads.CoreWorkload

fieldcount=1
fieldlength=131072
field_len_dist=fixed
readallfields=true
writeallfields=true
splitlsm.enable_gc=false

readproportion=0
updateproportion=0
deleteproportion=1
insertproportion=0
scanproportion=0
readmodifywriteproportion=0

requestdistribution=zipfian
