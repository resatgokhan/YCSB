# Mixed workload: 10% read, 50% update, 40% delete
recordcount=100000
operationcount=100000
workload=com.yahoo.ycsb.workloads.CoreWorkload

fieldcount=1
fieldlength=100
field_len_dist=constant
readallfields=true
writeallfields=true

readproportion=0.1
updateproportion=0.5
deleteproportion=0.4
insertproportion=0
scanproportion=0
readmodifywriteproportion=0

requestdistribution=zipfian
