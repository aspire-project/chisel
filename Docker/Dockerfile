FROM debian:sid 

RUN apt-get update && apt-get install -y sudo
RUN sudo apt-get install -y libmlpack-dev \
    clang libclang-dev llvm-dev llvm cmake git \
    libspdlog-dev nlohmann-json-dev

RUN git clone https://github.com/aspire-project/chisel
RUN git clone https://github.com/aspire-project/chiselbench

RUN mkdir -p chisel/build && cd chisel/build && CXX=clang cmake .. && make

ENV PATH /usr/local/sbin:/usr/local/bin:/usr/sbin:/usr/bin:/sbin:/bin:/chisel/build/bin
ENV CC clang
ENV CHISEL_BENCHMARK_HOME /chiselbench
