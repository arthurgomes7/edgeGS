# 🌊 FloodDrone ESP32 + Node-RED

Projeto de monitoramento ambiental utilizando **ESP32** com sensores de **temperatura, umidade e distância**, que envia dados via **MQTT** para uma interface de visualização feita no **Node-RED**.

## 📡 Visão Geral

Este sistema simula um drone equipado com sensores para:

- Medir **umidade** e **temperatura** (DHT22)
- Detectar **nível da água** (Sensor Ultrassônico HC-SR04)
- Enviar os dados via **protocolo MQTT** para um dashboard no Node-RED
- Acionar um LED de alerta em condições críticas

## 🔧 Componentes

- ESP32
- Sensor DHT22
- Sensor Ultrassônico HC-SR04
- LED (alarme visual)
- Broker MQTT (HiveMQ público)
- Node-RED (Dashboard para visualização dos dados)

## 📶 Funcionamento

1. O ESP32 conecta-se à rede Wi-Fi e ao broker MQTT (`broker.hivemq.com`)
2. A cada 5 segundos, ele:
   - Lê temperatura e umidade do DHT22
   - Mede distância com o sensor ultrassônico
   - Publica os dados no tópico MQTT: `sensor/monitoramento`
   - Acende o LED se a **umidade for maior que 70%** e a **distância menor que 50cm**

Vídeo de apresentação: https://youtu.be/3QaJ8nbNtoI

## Integrantes
Arthur Gomes - RM 560771
Matheus Siroma - RM 560248
Pedro Estevam - RM 560642

### Exemplo de JSON publicado:
```json
{
  "umidade": 74.2,
  "temperatura": 26.5,
  "distancia": 32
}
