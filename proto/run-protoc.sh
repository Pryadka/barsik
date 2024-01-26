protoc -I ./ --grpc_out=../generated/ --plugin=protoc-gen-grpc=`which grpc_cpp_plugin` ./turret-service.proto
protoc -I ./ --cpp_out=../generated ./turret-service.proto