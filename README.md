# IOCP 서버

## 프로젝트 소개

- 멀티스레드 IOCP 게임 서버
- 프로젝트 인원 : 1명

## 프로젝트 기간

- 2022.07 ~ 2023.01

## 라이브

- 동시 접속 550명 문제 없이 돌아갔음(실제 라이브 환경, CPU 점유율 0.1%)
- 더 감당이 가능했지만 들어오는 유저가 없었다....
 
## 다양한 시도 및 구현

- `Select`방식으로 했었는데 더 많은 소켓을 감당할 수 있는 서버를 찾다가 `IOCP`로 방향을 잡음
- `spdlog`를 도입해서 통신에 대한 지속적인 로그 저장

## 문제와 해결법

- 패킷이 빠르게 전송되다 보면 하나로 묶여서 들어오는 경우 발견
  - 해결법 : 패킷마다 사이즈를 포함해서 해당 패킷 사이즈만 처리하는 방식으로 구현

<br>

- 클라이언트는 연결이 끊겼더라도 서버에서 안 끊겨서 계속 소켓을 차지하는 경우가 있음
  - 해결법 : KEEPALIVE를 사용해서 주기적으로 클라이언트에게 패킷을 보내 확인

<br>

- 소켓이 많이 접속해있는 경우 트래픽이 늘어남
  - 원인 : 지금 패킷 보내는 방식이 고정 배열을 사용해서 최대치 기준으로 패킷을 보내고 있기 때문
  - 해결법 : 메모리 스트림을 구현해 필요한 데이터만 버퍼에 쌓아서 전송하게 구현 중

## 코드 부분

- 패킷 전송
https://github.com/reoul/ISEGYE-IDOL-GAME-SERVER/blob/3997fbb79f176924281464b41d2ede94621520bc/Server/Server/Server.cpp#L418-L434

<br>

- 소켓 연결 해제
https://github.com/reoul/ISEGYE-IDOL-GAME-SERVER/blob/3997fbb79f176924281464b41d2ede94621520bc/Server/Server/Server.cpp#L221-L246

<br>

- 패킷 가공
https://github.com/reoul/ISEGYE-IDOL-GAME-SERVER/blob/3997fbb79f176924281464b41d2ede94621520bc/Server/Server/Server.cpp#L248-L302

<br>

- 패킷 작업
https://github.com/reoul/ISEGYE-IDOL-GAME-SERVER/blob/3997fbb79f176924281464b41d2ede94621520bc/Server/Server/Server.cpp#L315-L416

<br>

- 워커 스레드
https://github.com/reoul/ISEGYE-IDOL-GAME-SERVER/blob/3997fbb79f176924281464b41d2ede94621520bc/Server/Server/Server.cpp#L77-L212

<br>

- 배틀 상대 겹치지 않고 구해주는 로직
https://github.com/reoul/ISEGYE-IDOL-GAME-SERVER/blob/d9abc3044ad2678d5b440b42d84a5b79e3a7cd26/Server/Server/BattleManager.cpp#L21-L132

<br>

- 아이템이 장착하지 않은 경우 UnUsingInventory에 있는 유효 아이템 장착
https://github.com/reoul/ISEGYE-IDOL-GAME-SERVER/blob/d9abc3044ad2678d5b440b42d84a5b79e3a7cd26/Server/Server/Client.cpp#L144-L179

<br>

- 해당 방에 있는 클라이언트들의 장착된 아이템을 랜덤으로 순서를 결정 
https://github.com/reoul/ISEGYE-IDOL-GAME-SERVER/blob/d9abc3044ad2678d5b440b42d84a5b79e3a7cd26/Server/Server/Room.cpp#L88-L196

<br>

- 매칭 관련 코드
https://github.com/reoul/ISEGYE-IDOL-GAME-SERVER/blob/d9abc3044ad2678d5b440b42d84a5b79e3a7cd26/Server/Server/ServerQueue.cpp#L16-L67

<br>

- 매칭 인원 수를 만족하면 방을 만들어 줌
https://github.com/reoul/ISEGYE-IDOL-GAME-SERVER/blob/d9abc3044ad2678d5b440b42d84a5b79e3a7cd26/Server/Server/ServerQueue.cpp#L69-L97
