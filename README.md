# MC833

1. ./server port                    # Servidor é iniciado na porta <port>
2. ./client ip port name            # Cliente name é iniciado, conectado ao servidor localizado em ip:port,
registrado no servidor, e recebe uma sessão vazia (denotada aqui por '$[name]')
3. $[name] SEND peer_name msg       # Cliente name envia mensagem msg para cliente peer_name
4. $[name] CREATEG group_name       # Cliente name cria grupo group_name
5. $[name] JOING group_name         # Cliente name entra no grupo group_name
6. $[name] SENDG group_name msg     # Cliente name envia mensagem para grupo group_name
7. $[name] WHO                      # Cliente name pede ao servidor lista dos usuários, e ao recebê-la, imprime na tela
8. $[name] EXIT                     # Cliente name faz logout
