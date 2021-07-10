# DEFINIÇÕES DO SEGUIDOR DE LINHA PID
numero erro = 0
numero p = 0
numero i = 0
numero integral = 0
numero d = 0
numero lastErro = 0
numero PID = 0
numero correcaoE = 0
numero correcaoD = 0
numero velocidade = 125

numero calibracao = 0.6

# HORÁRIO DESCRITO EM NUMERO COM VÍRGULA
#   => o número inteiro é a hora (12:00 será 12)
#   => o número após o ponto será os minutos, porém descritos como uma fração
#      do horário (9:30 será 9.5; 10:45 será 10.75; e assim por diante)
numero horario = 12

tarefa segueLinha {

  # TODO => verificar novamente quais horários causam problemas (testar em RAMPA)
  #         --> (RESOLVIDO DIFERENTE) 10h foi encontrado problema de uma pequena
  #             diferença entre os sensores quando SUBINDO EM RAMPA
  #             (em ambas as direções) 

  # VERIFICA O HORÁRIO DO DIA PARA CORRIGIR CASO HAJA SOMBRA
  # E DEFINE O ERRO COM BASE NA CORREÇÃO
  se (((7 < horario) e (horario < 7.75)) ou ((16.25 < horario) e (horario < 17.25)) ) entao {
    se ( (75 < direcao()) e (direcao() < 105) ) entao {
        erro = (((luz(2) - luz(3)) - 25) * calibracao)
    } senao se ( (255 < direcao()) e (direcao() < 285) ) entao {
        erro = (((luz(2) - luz(3)) + 25) * calibracao)
    } senao {
        erro = ((luz(2) - luz(3)) * calibracao)
    }
  } senao {
    erro = ((luz(2) - luz(3)) * calibracao)
  }
  escrevernumero(1, erro)
  # DEFININDO VALORES DO PID
  p = erro * 50
  integral = integral + erro
  i = integral * 0.0001
  d = (erro - lastErro) * 100
  lastErro = erro

  PID = p + i + d

  # CORRIGINDO A MOVIMENTAÇÃO
  correcaoE = (velocidade + PID)
  correcaoD = (velocidade - PID)

  mover(correcaoE, correcaoD)
}

# tarefa para debug somente
tarefa parei {
  enquanto (verdadeiro) farei {
    parar()
  }
}

inicio
  # CALIBRAÇÃO DA COR
  ajustarcor(30)

  # TODO => remover comentários
  # TODO (em processo) => adicionar verificação para, em determinada hora do dia, calibrar melhor

  # TODO:
  #     ideia para detectar que está na ÁREA DE RESGATE
  #     => ultra(2) < 33 e ultra(3) < 33 e inclinação <= 345
  #        => se ultra(1) < 500 --> chegou na ÁREA DE RESGATE

  enquanto (verdadeiro) farei {
    # EXPLICAÇÃO DAS VERIFICAÇÕES
    #   1a verificação obstaculo
    #   2a verificação cores => 
    #     verde nos sensores 1 2 3 e 4
    #     preto nos sensores 1 e 4
    #     segue linha

    se ((ultra(1) <= 10) e (cor(5) != "BRANCO")) entao {
      # RESUMO:
      # quando o sensor ultra da frente ver um obstaculo e
      # o sensor de cor frontal verificar que é diferente de BRANCO
      #   ele vira para a DIREITA, segue reto, vira para a ESQUERDA
      #   e verifica com o sensor ultra da esquerda quando passar pelo obstáculo
      #     quando passou, segue um pouco mais a frente e vira a ESQUERDA para
      #     retornar à linha
      #       ele segue reto até ver preto e depois vai mais um pouco,
      #       virando 45° e depois até ver preto (para a DIREITA) 
      escrever(1, "encontrei um obstaculo")

      rotacionar(500, 90)
      frenterotacao(200, 16)
      rotacionar(500, negativo(90))

      enquanto (ultra(3) > 10) farei {
        frente(200)
      }
      enquanto (ultra(3) < 10) farei {
        frente(200)
      }
      frenterotacao(200, 15)
      rotacionar(500, negativo(90))

      enquanto ((cor(1) == "PRETO") ou (cor(2) == "PRETO") ou (cor(3) == "PRETO") ou (cor(4) == "PRETO")) farei {
        frente(100)
      }

      frenterotacao(100, 16)
      rotacionar(500, 45)

      enquanto (cor(2) != "PRETO") farei {
        direita(500)
      }
    } senao {
      # RESUMO:
      # PRIMEIRO -> VERIFICA SE HÁ VERDE EM ALGUM SENSOR DE COR
      #     se houver:
      #         para por 0.25 segundos para conferir aonde está a cor
      #         se em ambos os lados:
      #             virá 180°
      #         se em somente um dos lados:
      #             vai por 14 rotações para a frente, vira 20° para o lado respectivo
      #             e continua virando até ver preto com o sensor respectivo:
      #               se estiver virando para a direita, sensor 2; se para a esquerda, sensor 3
      #         depois de realizar o movimento, retorna 4 rotações e volta a seguir linha
      #     se não houver -> VERIFICA SE HÁ PRETO NOS EXTERNOS (1 e 4):
      #         se houver -> para e espera 0.5 segundos para verificar aonde está o preto
      #             se em ambos os lados:
      #                 segue reto e volta a seguir linha
      #             se somente em um lado:
      #                 vai 3 rotações para a ferente e vira 4° para a direita
      #                 entra em um ciclo para verificar se há preto na frente
      #                   se houver, volta os 4° e sai do ciclo
      #                   se não houver, volta os 4° e vai mais a frente, vira 90°
      #                     para o lado respectivo e retorna 6 rotações, para então
      #                     verificar se está na linha:
      #                       virando o lado oposto até ver preto
      #                       e dps para o lado respectivo até ver preto
      #                     enfim: sai do ciclo e volta a seguir linha!
      #         se não houver -> segue linha (normal)

      se (((cor(1) == "VERDE") ou (cor(2) == "VERDE")) ou ((cor(3) == "VERDE") ou (cor(4) == "VERDE"))) entao {
        escrever(1, "verde")
        parar()
        esperar(250)
        se (((cor(1) == "VERDE") ou (cor(2) == "VERDE")) e ((cor(3) == "VERDE") ou (cor(4) == "VERDE"))) entao {
          escrever(1, "verde com verde")
          rotacionar(200, 180)
        } senao se ((cor(1) == "VERDE") ou (cor(2) == "VERDE") e ((cor(3) != "VERDE") e (cor(4) != "VERDE"))) entao {
          escrever(1, "achei verde na direita")
          frenterotacao(100, 14)
          rotacionar(500, 20)
          enquanto (cor(3) != "PRETO") farei {
            escrever(2, "procurando preto...")
            direita(750)
          }
        } senao se ((cor(1) != "VERDE") e (cor(2) != "VERDE") e ((cor(3) == "VERDE") ou (cor(4) == "VERDE"))) entao {
          escrever(1, "achei verde na esquerda")
          frenterotacao(100, 14)
          rotacionar(500, negativo(20))
          enquanto (cor(2) != "PRETO") farei {
            escrever(2, "procurando preto...")
            esquerda(750)
          }
        }
        trasrotacao(100, 4)
        limparconsole()
      } senao se ((cor(1) == "PRETO") ou (cor(4) == "PRETO")) entao {
        parar()
        esperar(500)
        se ((cor(1) == "PRETO") e (cor(4) == "PRETO")) entao {
          escrever(1, "preto nos dois")
          frenterotacao(100, 3)
        } senao se ((cor(1) == "PRETO") e (cor(4) != "PRETO")) entao {
          escrever(1, "preto direita")
          frenterotacao(100, 3)
          rotacionar(500, 4)

          enquanto (verdadeiro) farei {
            se ((cor(2) == "PRETO") ou (cor(3) == "PRETO")) entao {
              escrever(2, "achei preto na frente")
              rotacionar(500, negativo(4))
              interromper()

            } senao {
              escrever(2, "NÃO achei na frente")
              rotacionar(500, negativo(4))

              frenterotacao(100, 8)
              rotacionar(500, 90)
              trasrotacao(100, 6)

              enquanto (cor(2) != "PRETO") farei {
                escrever(2, "virando esquerda...")
                esquerda(250)
              }
              enquanto (cor(3) != "PRETO") farei {
                direita(250)
              }
              limparlinha(2)
              interromper()
            }
          }
        } senao se ((cor(1) != "PRETO") e (cor(4) == "PRETO")) entao {
          escrever(1, "preto esquerda")
          frenterotacao(100, 3)
          rotacionar(500, 4)

          enquanto (verdadeiro) farei {
            se ((cor(2) == "PRETO") ou (cor(3) == "PRETO")) entao {
              escrever(2, "achei preto na frente")
              rotacionar(500, 4)
              interromper()

            } senao {
              escrever(2, "NÃO achei preto na frente")
              rotacionar(500, negativo(4))

              frenterotacao(100, 8)
              rotacionar(500, negativo(90))
              trasrotacao(100, 6)

              enquanto (cor(3) != "PRETO") farei {
                escrever(2, "virando direita...")
                direita(250)
              }
              enquanto (cor(2) != "PRETO") farei {
                esquerda(250)
              }

              limparlinha(2)
              interromper()
            }
          }
        }
        limparconsole()
      }

      # SEGUE LINHA:
      # PARA SEGUIR LINHA:
      #     -> caso esteja em rampa OU esteja reto (nas direções NORTE SUL LESTE OESTE):
      #         -> se os dois sensores da frente verem branco = segue em frente
      #         -> se algum sensor ver algo diferente = segue linha
      #     -> caso esteja em qualquer angulo diferente ou não esteja inclinado:
      #         -> segue linha normalmente
      se ((15 < inclinacao()) e (inclinacao() < 345)) entao {
        velocidade = 100
        se ((cor(2) == "BRANCO") e (cor(3) == "BRANCO")) entao {
          escrever(3, "branco branco subindo")
          frente(velocidade)
        } senao {
          escrever(3, "seguindo subindo")
          segueLinha()
        }
      } senao se ((((0 <= direcao()) e (direcao() < 7)) ou ((353 < direcao()) e (direcao() <= 360))) ou ((83 < direcao()) e (direcao() < 97)) ou ((173 < direcao()) e (direcao() < 187)) ou ((263 < direcao()) e (direcao() < 277))) entao {
        velocidade = 125
        se ((cor(2) == "BRANCO") e (cor(3) == "BRANCO")) entao {
          escrever(3, "branco branco quando reto")
          frente(velocidade)
        } senao {
          escrever(3, "seguindo quando reto")
          segueLinha()
        }
      } senao {
        velocidade = 125
        escrever(3, "seguindo normal")
        segueLinha()
      }
    }
  }  
fim