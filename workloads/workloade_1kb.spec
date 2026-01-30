# Workload E (1KB): 95% scan, 5% insert (zipfian)
recordcount=104857600
operationcount=104857600
workload=com.yahoo.ycsb.workloads.CoreWorkload

fieldcount=1
fieldlength=1024
field_len_dist=fixed
readallfields=true
writeallfields=true
splitlsm.enable_gc=false

readproportion=0
updateproportion=0
deleteproportion=0
insertproportion=0.05
scanproportion=0.95
readmodifywriteproportion=0

requestdistribution=zipfian
scanlengthdistribution=uniform
maxscanlength=100
