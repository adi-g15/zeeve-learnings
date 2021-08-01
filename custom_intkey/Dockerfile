# This is a multi-stage dockerfile, to reduce final image size
FROM rust:1.53 AS BUILD_STAGE

RUN apt update
RUN apt install gcc curl libzmq3-dev libssl-dev pkg-config protobuf-compiler -y

# Copy & Build the "Client" & "Transaction processor"
# Client already in sawtooth-shell
# COPY client_cli /tmp/client
# WORKDIR /tmp/client
# RUN cargo build --release

COPY intkey-tp /tmp/processor
WORKDIR /tmp/processor
RUN cargo build --release

FROM hyperledger/sawtooth-shell:nightly

COPY --from=BUILD_STAGE /tmp/processor/target/release/custom-intkey-tp /usr/bin/

CMD bash
