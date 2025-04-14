ARG tag=latest
FROM ubuntu:${tag}

LABEL maintainer="azip10@gmail.com"

LABEL Description="Provide Docker images for QuantLib's CI builds on Linux"

RUN apt-get update \
 && DEBIAN_FRONTEND=noninteractive apt-get install -y build-essential wget libbz2-dev autoconf automake libtool ccache cmake clang git \
 && apt-get clean \
 && rm -rf /var/lib/apt/lists/*

ENV boost_version=1.87.0
ENV boost_dir=boost_1_87_0


RUN wget https://archives.boost.io/release/${boost_version}/source/${boost_dir}.tar.gz \
    && tar xfz ${boost_dir}.tar.gz \
    && rm ${boost_dir}.tar.gz \
    && cd ${boost_dir} \
    && ./bootstrap.sh \
    && ./b2 --without-python --prefix=/usr -j 4 link=shared runtime-link=shared install \
    && cd .. && rm -rf ${boost_dir} && ldconfig

CMD ["bash"]

