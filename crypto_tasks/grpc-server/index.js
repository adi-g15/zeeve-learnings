const PROTO_PATH = __dirname + "/certificate.js";
const grpc = require("@grpc/grpc-js");
const ProtoLoader = require("@grpc/proto-loader");

/*Why these options only?... As stated by the grpc docs, this is to be similar to ProtoLoader.load behaviour*/
const package_definition = ProtoLoader.loadSync(
	PROTO_PATH,
	{
		keepCase: true,
		longs: String,
		enums: String,
		defaults: true,
		oneofs: true
	}
)
const package_descriptor = grpc.loadPackageDefinition( package_definition );

/*Now we start the actual working... this `package_descriptor` has the complete package heirarchy*/

