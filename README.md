## 项目简介
随着物联网、云计算、大数据技术的发展，物联网技术对农业生产监测控制提供了可行性方案，
其低廉的感知成本，自适应的网络拓扑，全面感知和计算等特性，在智能农业领域大放异彩。 
云计算、大数据等技术也推进农业管理数字化和现代化，促进农业管理高效和透明，农资采购和农产品流通等数据将会得到实时监测和传递，有效解决信息不对称问题。  
Agricultural-IOT-System将提供了一套可靠、全面的整体解决方案，解决方案集生产环境监测、智能设备控制、农业生产指导、和农产品互联网销售于一体。  

## 系统总体设计  
智能农业云平台主要功能为：环境监测功能、智能控制功能、基于大数据分析的农技生产指导功能、互联网营销功能和容灾应急功能。
云平台同时分为两个部分进行开发：监测控制系统和交互界面系统。  
<div align=center>
<img src="https://github.com/efishliu/Agricultural-IOT-System/blob/master/image/%E7%B3%BB%E7%BB%9F%E6%80%BB%E4%BD%93%E8%AE%BE%E8%AE%A1%E5%9B%BE.jpg?raw=true" width = 50% height = 50% />
</div>  


## 系统功能设计与实现  
* **监测子系统**  
通过部署在农业大棚内的各种传感器，全面展示和监测农业大棚的大气环境、土壤环境、水质环境、作物长势、设备运行状态、病虫害情况等，并将监测到的数据传输至云平台，进行存储、分析，并根据环境进行智能控制处理，发出相应的控制信息实现智能控制。并根据状态信息更新交互界面。  
<div align=center>
<img src="https://github.com/efishliu/Agricultural-IOT-System/blob/master/image/%E9%9C%80%E6%B1%821.jpg?raw=true" width = 50% height = 50% />
</div>  

[监测子系统传感器功能与组网测试](https://github.com/efishliu/Agricultural-IOT-System/tree/master/SerialApp)

* **控制子系统**  
控制子系统由控制设备类型、控制方式、控制信息格式三部分组成。实现多种方式对设备进行远端控制。  
[监测子系统传感器功能测试](https://github.com/efishliu/Agricultural-IOT-System/tree/master/Stand-alone-cc2530)  

* **云平台分析子系统**  
云平台分析子系统通过利用云计算技术，采用软件即服务模式（Saas）进行平台的搭建，数据集中到云数据中心统一存储与处理。并搭建分布式计算框架进行人工智能分析。云平台分析子系统主要分为两部分：即基于监测数据的智能控制分析和基于大数据的农业生产指导分析。  
1.基于监测数据的智能控制分析：分析各种传感器、控制器规则策略，建立智能分析策略模型，通过基于传感器监测数据，进行智能分析，实现农业生产的智能预警和联动控制。  
[基于监测数据的智能控制分析与实现](https://github.com/efishliu/Agricultural-IOT-System/tree/master/Intelligent-Agricultural/project)  
2.基于大数据的农业生产指导分析：基于监测数据和其他可靠数据，通过大数据平台和人工智能算法，对农产品的需求、作物生长影响因素等做出分析，提供基于大数据的准确的农业生产指导。大数据平台架构如下图：  
<div align=center>
<img src="https://github.com/efishliu/Agricultural-IOT-System/blob/master/image/%E9%9C%80%E6%B1%822.jpg?raw=true" width = 50% height = 50% />
</div>  

* **用户交互子系统**  
1.监测控制状态界面:监测界面由APP端、web端、本地端多端进行展示，与用户进行交互。监测界面分为数据类型和数据展示方式两个部分。如下图：  
<div align=center>
<img src="https://github.com/efishliu/Agricultural-IOT-System/blob/master/image/%E9%9C%80%E6%B1%822.jpg?raw=true" width = 40% height = 40% />  
<img src="https://github.com/efishliu/Agricultural-IOT-System/blob/master/image/%E9%9C%80%E6%B1%822.jpg?raw=true" width = 40% height = 40% /></div>    

Web端实现代码:[Web](https://github.com/efishliu/Agricultural-IOT-System/tree/master/Intelligent-Agricultural/web)  
PC端实现代码:[Projects](https://github.com/efishliu/Agricultural-IOT-System/blob/master/Intelligent-Agricultural/project/Display.py)  

2.互联网销售界面:互联网销售界面采用基于APP 的交互方式，为使用者提供良好的交互界面，实现农资采购和农产品互联网销售的功能。互联网销售界面分为三个部分：供应信息、采购信息、账号管理。如下图：


* **容灾子系统**  

