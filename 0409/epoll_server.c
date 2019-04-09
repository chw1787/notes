#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/select.h>
#include <netinet/in.h>
#include <poll.h>

#include <stdio.h>
#include <string.h>

// poll: System V에서 처음 제공된 시스템 콜입니다.
//   => select의 사용상의 단점을 보완한다.

// 1. nfds(최대값)을 관리할 필요가 없다.
// 2. 1024개의 한계가 없다.
// 3. select에 비해서 효율적으로 동작한다.

// 단점
// 1. 디스크립터 배열을 직접 관리해야 한다.
// 2. 비효율적이다.
//    : 하나의 디스크립터에서 이벤트가 발생하였다고 하더라도,
//      해당 디스크립터를 검색해야 한다.


//epoll: 2.6커널 에서 도입되었습니다.
// => poll 사용상의 단점을 보완하고 성능을 개선한다.
// 1) epoll 저장소를 토해서 디스크립터 배열을 커널이 관리 한다.
// epoll_ctl 함수를 통해 디스크립터를 등록 해지 수정할 수 있다.

// 2) poll이 반환하면 이벤트가 발생한 디스크립터를 찾기 위해 전체 배열을 순회해야한다.
// epoll 은 이벤트가 발생한 디스크립터 배열을 반환한다. 
// 



int main() {
	int ssock = socket(PF_INET, SOCK_STREAM, 0);
	if (ssock == -1) {
		perror("socket");
		return 1;
	}

	int option = 1;
	setsockopt(ssock, SOL_SOCKET, SO_REUSEADDR, &option, sizeof option);

	struct sockaddr_in saddr = {0, };
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = INADDR_ANY;
	saddr.sin_port = htons(5000); 
	if (bind(ssock, (struct sockaddr *)&saddr, sizeof saddr) == -1) {
		perror("bind");
		return 1;
	}

	if (listen(ssock, SOMAXCONN) == -1) {
		perror("listen");
		return 1;
	}
	// 1. 저장소를 생성해야한다. 
	// epoll의 create를 생성해야한다. 
	// epoll_create와 epoll_create1 이 존재한다. 
	// 위에 꺼는 쓰지 말아야 한다. 잘못만든 것이다. 
	// 내가 만약에 10000개로 하겠다. 하면 size 를 쓰면안된다.
	// epoll_create1을 써야한다. 
	// epoll_create()는 더이상 사용되지 않습니다.
	// flag 는 default 로 만들 것이다. 
	int efd = epoll_create1(0);
	if (efd == -1) {
		perror("epoll_create()");
		return -1;
	}
		

	struct epoll_event event = {0, };
	//union 왜 쓸까요 4개중에 하나만 상호 배타적으로 사용하게 된다.
	// 메모리 낭비를 방지하기 위해서 사용한다.
	// 상당히 유연한 상태로 되어있습니다.
	// 진짜 쓸때는 void *를 사용하는 특유의 기법이 있습니다. 
	// generic 연결리스트  

	event.events = EPOLLIN;
	event.data.fd = ssock;
	epoll_ctl(EPOLL_CTL_ADD, ssock, &event);
	
	
	//epoll_ctl
	//poll의 코드이다 
	// ---------------------
	struct pollfd fds[4096];
	fds[0].fd = ssock;
	fds[0].events = POLLIN;
	int nfds = 1;
	// ---------------------
	// read / accept: POLLIN
	// write: POLLOUT

	int i;
	int ret;
	char buf[512];
	int compress = 0;
	struct poll_event revents[1024];

	while (1) {
		n = epoll_wait(efd, revents, 1024, -1);
		//poll(fds, nfds, -1);

		for (i = 0; i < nfds; ++i) {
			if (fds[i].revents & POLLIN) {
				if (fds[i].fd == ssock) {  // accept
					struct sockaddr_in caddr = {0, };
					socklen_t caddrlen = sizeof caddr;
					int csock = accept(ssock, (struct sockaddr *)&caddr, &caddrlen);
					if (csock == -1) {
						perror("accept");
						return 1;
					}

					printf("client: %s\n", inet_ntoa(caddr.sin_addr));
					evnet.data.df = csock;
					event.events = EPOLLIN;
					//epoll_ctl(efd, EPOLL_CTL 
					// 감시하고자 하는 디스크립터를 등록한다.
					//-----------------
					fds[nfds].fd = csock;
					fds[nfds].events = POLLIN;
					nfds++;
					//-----------------
				} else {                   // read
					int fd = revent[i].data.fd;
					ret = read(fd, buf, sizeof buf);			
					if (ret <= 0) {
						printf("연결이 종료되었습니다...\n");
						close(fd);
							
						epoll_ctl(efd, EPOLL_CTL_EDL, fd, NULL);;



						fds[i].fd = -1;
						compress = 1;
					}
					write(fd, buf, ret);
				}
			}
		}

		// poll의 핵심: 감시하고자 하는 디스크립터 배열을 직접 관리해야 한다.
		//  새로운 디스크립터를 추가하고, 삭제하는 로직을 작성해야 한다.
		if (compress) {
			int n = 0;
			struct pollfd temp[4096];
			
			for (i = 0; i < nfds; ++i) {
				if (fds[i].fd != -1) {
					temp[n++] = fds[i];
				}
			}

			memcpy(fds, temp, n * sizeof(struct pollfd));
			nfds = n;
			compress = 0;
		}
	}

	close(ssock);
	return 0;
}







