<template>
  <div class="home-container">
    <el-card shadow="always" style="margin-bottom:30px;">
      <el-form ref="subscription" hide-required-asterisk size="small" label-position="top" :model="subscription">
        <el-row :gutter="20">
          <el-col :span="8">
            <el-form-item prop="SelectMac" label="選擇鎖定mac">
              <el-select v-model="TestMessage.SelectMac">
                <el-option v-for="(item, index) in macs" :key="index" :label="macs[index]" :value="macs[index]">
                </el-option>
              </el-select>
            </el-form-item>
          </el-col>
        </el-row>
      </el-form>
    </el-card>
    <!-- 範例 -->
    <el-card shadow="always" style="margin-bottom:30px;">
      <div class="emq-title">
        <tr>
          {{ TestMessage.topic }}
        </tr>
        <tr>
          {{ TestMessage.topic2 }}
        </tr>
        <tr>
          {{ TestMessage.message[0] }}
        </tr>
        <tr>
          {{ TestMessage.message[1] }}
        </tr>
        <tr>
          {{ TestMessage.message[2] }}
        </tr>
      </div>
      <el-button type="success" size="small" class="conn-btn" style="margin-right: 20px;" @click="DrawCanvas">
        CanvasTest
      </el-button>
    </el-card>
    <!-- 顯示ESP座標 -->
    <el-card shadow="always" style="margin-bottom:30px;">
      <div class="emq-title">
        Station Position
        <tr>
          esp0: {{ StationsInfo.esp1 }}
        </tr>
        <tr>
          esp1: {{ StationsInfo.esp2 }}
        </tr>
        <tr>
          esp2: {{ StationsInfo.esp3 }}
        </tr>
      </div>

    </el-card>
    <!-- 測試回覆 -->
    <el-card style="margin-bottom:30px;">
      <div class="emq-title">
        (測試) 計算位置:
      </div>
      <tr>
        <span> {{ TestMessage.SelectMac }}</span>
      </tr>
    </el-card>
    <!-- 室內定位圖 -->
    <el-card style="margin-bottom:20px;">
      <div class="emq-title">
        室內定位圖
      </div>
    </el-card>
    <!-- IndoorPosition IMG -->
    <div style="width:400px">
      <!-- <img :src="indoor" class="img" alt="" /> -->
      <canvas id="mycanvas" width='1100' height='1000' class="my-canvas"></canvas>
    </div>

  </div>
</template>

<script>
import mqtt from 'mqtt'
import trilat from 'trilat'

//引入圖片 indoor,target
import indoor from "@/assets/img/indoor.png"
import target from "@/assets/img/target.png"

export default {
  name: 'Home',
  data() {
    return {
      TestMessage: {
        topic: 'Topic: indoor/#',
        topic2: 'Topic: indoor/',
        message: [
          '{"station":"esp1","info":[{"mac":"test1","rssi":70},{"mac":"test2","rssi":50},{"mac":"test3","rssi":70}]}'
        ],
        SelectMac: ""
      }
      ,
      StationsInfo: {
        esp1: { x: 10, y: 12, DistanceMeter: 30, EnvironFator: 2.2 },
        esp2: { x: 20, y: 15, DistanceMeter: 30, EnvironFator: 2.2 },
        esp3: { x: 30, y: 15, DistanceMeter: 30, EnvironFator: 2.2 },
      },
      //偵測到的站點
      Stations: []
      ,
      //偵測到的beacon資訊
      beacons: {}
      ,
      //偵測到的mac
      macs: []
      ,
      PositionMessage: []
      ,
      //測試接收資訊
      user_receive: ""
      ,
      connection: {
        host: '127.0.0.1',
        port: 1884,
        endpoint: '',
        clean: true, // 保留會話
        connectTimeout: 4000, // Timeout
        reconnectPeriod: 4000, // 重新連接間隔
        clientId: 'MqttWeb_indoor',
      },
      subscription: {
        topic: 'indoor/#',
        qos: 0,
      },
      qosList: [
        { label: 0, value: 0 },
        { label: 1, value: 1 },
        { label: 2, value: 2 },
      ],
      client: {
        connected: false,
      },
    }
  },
  mounted() {
    //測試
    this.createConnection()
  },
  methods: {
    total(message) {
      this.MessageProcess(message)

      this.user_receive = this.beacons[this.macs]["esp1"]
      //["54:0e:72:66:99:59"]["esp1"]

      // let macs = this.macs
      // let beacons = this.beacons

      // for (let index = 0; index < macs.length; index++) {
      //   if (Object.keys(beacons[macs[index]]).length >= 3) {
      //     let [x, y] = this.CalculatePosition(beacons[macs[index]])
      //     this.PositionMessage[macs[index]] = { "x": x, "y": y }
      //   }
      // }
    },
    MessageProcess(message) {
      let msg = JSON.parse(message)

      // !== 相同型別才做比較
      //參考https://github.com/simonbogh/ESP32-iBeacon-indoor-positioning

      if (msg !== null) {
        for (let i = 0; i < msg.info.length; i++) {
          let station = msg.station
          let mac = msg.info[i].mac

          if (!this.Stations.includes(station)) {
            this.Stations.push(station)
          }
          if (typeof (this.beacons[mac]) !== 'object') {
            // Initialize
            this.beacons[mac] = []
            this.macs.push(mac)
          }
          if (this.beacons[mac][station] == null) {

            // Insert new record
            this.beacons[mac][station] = {
              rssi: parseInt(msg.info[i].rssi, 10)
            }
          }
          // -50 > -10 + -20 && -50 < -80 +20
          else if (this.beacons[mac][station].rssi + 15 >= msg.info[i].rssi && this.beacons[mac][station].rssi - 15 <= msg.info[i].rssi) {
            // Insert new record
            this.beacons[mac][station] = {
              rssi: parseInt((msg.info[i].rssi + this.beacons[mac][station].rssi) / 2, 10)
            }
          }




        }
      }
    },
    //CalculatePosition return [x, y]
    CalculatePosition(beacon) {
      function CalculateDistance(rssi, stations) {
        // Distance = 10^((abs(RSSI) - DistanceMeter) / (10 * EnvironFator))
        // RSSI - 接收訊號強度(負值)
        // DistanceMeter - 發收跟接收相距1米的訊號強度
        // EnvironFator - 環境衰減因子
        let DistanceMeter = stations.esp1.DistanceMeter
        let EnvironFator = stations.esp1.EnvironFator
        let Distance = Math.pow(10, (Math.abs(rssi) - DistanceMeter) / (10 * EnvironFator))

        return Distance;
      }

      let stations = this.StationsInfo
      let input = [
        [stations.esp1.x, stations.esp1.y, CalculateDistance(beacon.esp1.rssi, stations)],
        [stations.esp2.x, stations.esp2.y, CalculateDistance(beacon.esp2.rssi, stations)],
        [stations.esp3.x, stations.esp3.y, CalculateDistance(beacon.esp3.rssi, stations)]
      ]
      let output = trilat(input)

      this.user_receive = output
      //20 是DrawTablePhoto()中的BlockSize
      return [output[0], output[1]]

    },
    DrawCanvas() {

      let canvas = document.getElementById("mycanvas");//New canvas
      let context = canvas.getContext("2d"); //創立畫布

      canvas.setAttribute("width", 1100)
      canvas.setAttribute("height", 600)

      this.DrawTablePhoto(context)
      this.DrawTarget(context, this.PositionMessage)
    },
    DrawTablePhoto(context) {
      let x = 1100
      let y = 600
      let BlockSize = 20
      //畫座標 每格100
      for (let i = BlockSize; i <= x; i += 100) {
        context.fillText(i - BlockSize, i, BlockSize - 10)
      }
      for (let i = BlockSize; i <= y; i += 100) {
        context.fillText(i - BlockSize, 0, i)
      }
      //畫格子
      context.strokeStyle = 'black'
      context.lineWidth = 1
      //畫格子X軸
      for (let i = BlockSize; i <= x; i += BlockSize) {
        context.moveTo(i, BlockSize)
        context.lineTo(i, y)
        context.stroke()
      }
      //畫格子Y軸
      for (let i = BlockSize; i <= y; i += BlockSize) {
        context.moveTo(BlockSize, i)
        context.lineTo(x, i)
        context.stroke()
      }

    },
    DrawTarget(context, PositionMessage) {
      // let TargetImg = new Image() //New Object
      // TargetImg.src = target
      // context.drawImage(TargetImg, 520, 520, TargetImg.width, TargetImg.height)
      // TargetImg.onload = () => {
      // }
      let macs = this.macs

      for (let index = 0; index < macs.length; index++) {
        context.beginPath();
        context.arc(PositionMessage[macs[index]].x, PositionMessage[macs[index]].y, 10, 0, 2 * Math.PI);
        context.stroke();
        console.log(index + ":" + [PositionMessage[macs[index]].x, PositionMessage[macs[index]].y])
      }
    },
    DrawPhoto() {
      //console.log(this.message);
      let canvas = document.getElementById("mycanvas");//New canvas
      let context = canvas.getContext("2d"); //創立畫布

      let TargetImg = new Image(); //New Object
      let IndoorImg = new Image();

      TargetImg.src = target;
      IndoorImg.src = indoor;

      //load image
      IndoorImg.onload = () => {
        canvas.setAttribute("width", IndoorImg.width)
        canvas.setAttribute("height", IndoorImg.height)

        //繪製底圖
        context.drawImage(IndoorImg, 0, 0, IndoorImg.width, IndoorImg.height);
        //繪製目標圖
        for (let index = 0; index < this.UserData.length; index++) {
          context.drawImage(TargetImg, this.UserData[index].x, this.UserData[index].y, TargetImg.width, TargetImg.height);
        }
      };

    },

    //MQTT Connection,subscribe...
    createConnection() {
      // ws 未加密 WebSocket 連接
      // wss 加密 WebSocket 連接
      // mqtt 未加密 TCP 連接
      // mqtts 加密 TCP 連接
      const { host, port, endpoint, ...options } = this.connection
      const connectUrl = `ws://${host}:${port}${endpoint}`

      try {
        this.client = mqtt.connect(connectUrl, options)
        //Subscribe
        this.doSubscribe()
      } catch (error) {
        console.log('mqtt.connect error', error)
      }
      this.client.on('connect', () => {
        console.log('Connection succeeded!')
      })
      this.client.on('error', error => {
        console.log('Connection failed', error)
      })
      //進入點
      this.client.on('message', (topic, message) => {
        this.total(message)
      })
    },
    doSubscribe() {
      const { topic, qos } = this.subscription
      this.client.subscribe(topic, { qos }, (error, res) => {
        if (error) {
          console.log('Subscribe to topics error', error)
          return
        }
        console.log('Subscribe to topics res', res)
      })
    },
    destroyConnection() {
      if (this.client.connected) {
        try {
          this.client.end()
          this.client = {
            connected: false,
          }
          console.log('Successfully disconnected!')
        } catch (error) {
          console.log('Disconnect failed', error.toString())
        }
      }
    },
  },
}
</script>


<style lang="scss">
@import url('../assets/style/home.scss');

.home-container {
  max-width: 1100px;
  margin: 0 auto;

  .conn-btn {
    color: #fff;
    background-color: #00b173;
    font-size: 14px;
  }

  .publish-btn {
    margin-bottom: 20px;
    float: right;
  }

  .el-button--success {
    background-color: #34c388 !important;
    border-color: #34c388 !important;
    font-size: 14px !important;
  }

  .el-button--danger {
    background-color: #f5222d !important;
    border-color: #f5222d !important;
  }

  .el-form-item {
    &.is-error {

      .el-input__inner,
      .el-textarea__inner {
        box-shadow: 0 0 0 2px rgba(245, 34, 45, 0.2);
      }
    }

    &.is-success {

      .el-input__inner,
      .el-textarea__inner {
        border-color: #34c388 !important;
      }
    }
  }
}
</style>
