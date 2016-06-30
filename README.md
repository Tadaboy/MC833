# MC833

- [x] ./server port                    # Servidor é iniciado na porta <port>
- [x] ./client ip port name            # Cliente name é iniciado, conectado ao servidor localizado em ip:port,
registrado no servidor, e recebe uma sessão vazia (denotada aqui por '$[name]')
- [ ] $[name] SEND peer_name msg       # Cliente name envia mensagem msg para cliente peer_name
- [ ] $[name] CREATEG group_name       # Cliente name cria grupo group_name
- [ ] $[name] JOING group_name         # Cliente name entra no grupo group_name
- [ ] $[name] SENDG group_name msg     # Cliente name envia mensagem para grupo group_name
- [ ] $[name] WHO                      # Cliente name pede ao servidor lista dos usuários, e ao recebê-la, imprime na tela
- [ ] $[name] EXIT                     # Cliente name faz logout
