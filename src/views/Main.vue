<template>
  <div class="home-container">
    <!-- 顯示mac -->
    <!-- <el-card shadow="always" style="margin-bottom:30px;">
      <el-form ref="subscription" hide-required-asterisk size="small" label-position="top" :model="subscription">
        <el-row :gutter="20">
          <el-col :span="8">
            <el-form-item prop="Selectmac" label="選擇mac">
              <el-select v-model="selectmac">
                <el-option v-for="(item, index) in Message.UserMessage.MajorMinor" :key="index"
                  :label="Message.UserMessage.MajorMinor[index]" :value="Message.UserMessage.MajorMinor[index]">
                </el-option>
              </el-select>
            </el-form-item>
          </el-col>
        </el-row>
      </el-form>
    </el-card> -->
    <div class="emq-title">
      ESP Setting
    </div>
    <el-card shadow="always" style="margin-bottom:30px;">
      <div class="emq-title">
        ESP1 Setting
      </div>
      <el-row :gutter="20">
        <el-col :span="4">
          <div>X座標:</div>
          <el-input v-model="Message.StationsInfo.esp1.x"></el-input>
        </el-col>
        <el-col :span="4">
          <div>y座標:</div>
          <el-input v-model="Message.StationsInfo.esp1.y"></el-input>
        </el-col>
        <el-col :span="4">
          <div>DistanceMeter:</div>
          <el-input v-model="Message.StationsInfo.esp1.DistanceMeter"></el-input>
        </el-col>
        <el-col :span="4">
          <div>EnvironFator:</div>
          <el-input v-model="Message.StationsInfo.esp1.EnvironFator"></el-input>
        </el-col>
      </el-row>
      <div class="emq-title">
        ESP2 Setting
      </div>
      <el-row :gutter="20">
        <el-col :span="4">
          <div>X座標:</div>
          <el-input v-model="Message.StationsInfo.esp2.x"></el-input>
        </el-col>
        <el-col :span="4">
          <div>y座標:</div>
          <el-input v-model="Message.StationsInfo.esp2.y"></el-input>
        </el-col>
        <el-col :span="4">
          <div>DistanceMeter:</div>
          <el-input v-model="Message.StationsInfo.esp2.DistanceMeter"></el-input>
        </el-col>
        <el-col :span="4">
          <div>EnvironFator:</div>
          <el-input v-model="Message.StationsInfo.esp2.EnvironFator"></el-input>
        </el-col>
      </el-row>
      <div class="emq-title">
        ESP3 Setting
      </div>
      <el-row :gutter="20">
        <el-col :span="4">
          <div>X座標:</div>
          <el-input v-model="Message.StationsInfo.esp3.x"></el-input>
        </el-col>
        <el-col :span="4">
          <div>y座標:</div>
          <el-input v-model="Message.StationsInfo.esp3.y"></el-input>
        </el-col>
        <el-col :span="4">
          <div>DistanceMeter:</div>
          <el-input v-model="Message.StationsInfo.esp3.DistanceMeter"></el-input>
        </el-col>
        <el-col :span="4">
          <div>EnvironFator:</div>
          <el-input v-model="Message.StationsInfo.esp3.EnvironFator"></el-input>
        </el-col>
      </el-row>

    </el-card>

    <!-- 範例 -->
    <!-- <el-card shadow="always" style="margin-bottom:30px;">
      <div class="emq-title">
        <tr>
          {{ example.topic }}
        </tr>
        <tr>
          TEST: {{ example.message }}
        </tr>
      </div>
    </el-card> -->
    <!-- 測試回覆 -->
    <el-card style="margin-bottom:30px;">
      <div class="emq-title">
        (測試)
      </div>
      <el-row :gutter="20" style="margin-bottom:10px;">
        <el-col :span="4">
          <div>MQTT連接狀態: {{client.connected?"已連接":"未連接"}}</div>
        </el-col>

      </el-row>
      <el-row :gutter="20">
        <el-col :span="4">
          <div>測試消息: {{user_receive}}</div>
        </el-col>

      </el-row>
    </el-card>

    <!-- 室內定位圖 -->
    <el-card style="margin-bottom:20px;">
      <div class="emq-title">
        室內定位圖
      </div>
      <el-button type="success" size="small" class="conn-btn" style="margin-right: 20px;" @click="DrawCanvas">
        CanvasTest
      </el-button>
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
      example: {
        topic: 'Topic: indoor/#',
        message: '{"station":"esp1","info":[{"major":"1","minor":"1","rssi":-51},{"major":"1","minor":"1","rssi":-51},{"major":"1","minor":"1","rssi":-53},{"major":"1","minor":"1","rssi":-53},{"major":"1","minor":"1","rssi":-52}]}',
      },
      Message: {
        StationsInfo: {
          //設定esp初始數值
          esp1: { x: 100, y: 100, DistanceMeter: 50, EnvironFator: 2.2 },
          esp2: { x: 900, y: 500, DistanceMeter: 60, EnvironFator: 2.2 },
          esp3: { x: 200, y: 500, DistanceMeter: 60, EnvironFator: 2.2 },
        },
        UserMessage: {
          //偵測到的beacon資訊(已處理過) //ex. beacons[m_m][station]
          beacons: [],
          //偵測到的MajorMinor
          MajorMinor: [],
          //計算完位置訊息
          PositionMessage: [],
        },
      },
      //測試資訊
      user_receive: "",
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
    //創建MQTT連接
    this.createConnection()
  },
  methods: {
    total(message) {
      //處理JSON，RSSI取平均值，波動大不取。
      this.JsonProcess(message)

      //["54:0e:72:66:99:59"]["esp1"]


      let MajorMinor = this.Message.UserMessage.MajorMinor
      let beacons = this.Message.UserMessage.beacons

      for (let index = 0; index < MajorMinor.length; index++) {
        if (Object.keys(beacons[MajorMinor[index]]).length >= 3) {      //最少偵測到三台才計算
          //取出三台最近的esp和相對RSSI值(暫時只做三台)
          //let beacon = this.MinRssi_3Device(beacons[MajorMinor[index]])

          let [x, y] = this.CalculatePosition(beacons[MajorMinor[index]])

          //紀錄位置訊息，下次取平均
          this.Message.UserMessage.PositionMessage[MajorMinor[index]] = { "x": x, "y": y }
          this.DrawCanvas()

        }
      }

    },
    JsonProcess(message) {

      let msg = JSON.parse(message)

      // !== 相同型別才做比較
      //參考https://github.com/simonbogh/ESP32-iBeacon-indoor-positioning

      //RSSI取平均且波動太大不取

      if (msg !== null) {
        for (let i = 0; i < msg.info.length; i++) {
          //接收資料
          let station = msg.station
          let m_m = msg.info[i].major + "," + msg.info[i].minor
          let ReceiveRssi = msg.info[i].rssi
          //網站資料
          let beacons = this.Message.UserMessage.beacons
          let MajorMinor = this.Message.UserMessage.MajorMinor

          //判斷beacons[m_m]是否存在
          if (typeof (beacons[m_m]) !== 'object') {
            // Initialize
            beacons[m_m] = []
            MajorMinor.push(m_m)
          }
          if (beacons[m_m][station] == null) {

            // Insert new record
            beacons[m_m][station] = parseInt(ReceiveRssi, 10)

          }
          // -50 > -10 + -20 && -50 < -80 +20
          else if (beacons[m_m][station] + 20 >= ReceiveRssi && beacons[m_m][station] - 20 <= ReceiveRssi) {
            // Insert new record
            beacons[m_m][station] = parseInt((ReceiveRssi + beacons[m_m][station]) / 2, 10)
          }
        }
      }
    },
    //CalculatePosition return [x, y]
    CalculatePosition(beacon) {
      function CalculateDistance(rssi, station) {
        // Distance = 10^((abs(RSSI) - DistanceMeter) / (10 * EnvironFator))
        // RSSI - 接收訊號強度(負值)
        // DistanceMeter - 發收跟接收相距1米的訊號強度
        // EnvironFator - 環境衰減因子
        let DistanceMeter = station.DistanceMeter
        let EnvironFator = station.EnvironFator
        let Distance = Math.pow(10, (Math.abs(rssi) - DistanceMeter) / (10 * EnvironFator))

        return Distance;
      }

      let stations = this.Message.StationsInfo

      let input = [
        [stations.esp1.x, stations.esp1.y, CalculateDistance(beacon.esp1, stations.esp1)],
        [stations.esp2.x, stations.esp2.y, CalculateDistance(beacon.esp2, stations.esp2)],
        [stations.esp3.x, stations.esp3.y, CalculateDistance(beacon.esp3, stations.esp3)]
      ]

      let output = trilat(input)

      return [output[0], output[1]]

    },
    DrawCanvas() {

      let canvas = document.getElementById("mycanvas");//New canvas
      let context = canvas.getContext("2d"); //創立畫布

      canvas.setAttribute("width", 1100)
      canvas.setAttribute("height", 600)

      this.DrawTablePhoto(context)
      this.DrawTarget(context, this.Message.UserMessage.PositionMessage)
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
      let MajorMinor = this.Message.UserMessage.MajorMinor

      for (let index = 0; index < MajorMinor.length; index++) {
        context.beginPath();
        context.arc(PositionMessage[MajorMinor[index]].x + 20, PositionMessage[MajorMinor[index]].y + 20, 10, 0, 2 * Math.PI);
        context.stroke();
        console.log(index + ":" + [PositionMessage[MajorMinor[index]].x, PositionMessage[MajorMinor[index]].y])
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
