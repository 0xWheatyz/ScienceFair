# Two way Communication Link in the Sub-GHz Band

## **Question**

Can we create a communication link that transfers data over longer distances than WiFi while using less power?

## **What we want to do**

Our goal is to develop a low-power, long-range communication system that operates in the sub-GHz frequency range. This system would aim to provide greater transmission distances than traditional Wi-Fi or 2.4 GHz signals while consuming less power.

## **How we will do it**

We are using ESP32 microcontroller boards to control NRF905 transceiver modules. These modules enable wireless communication by transmitting and receiving data. To make this possible, we wrote custom software that enables both boards to send and receive text-based messages.

## **How we measure our success**

Our success will be measured by the ability of the two devices to wirelessly transmit and receive data over distances that exceed the range of typical Wi-Fi or 2.4 GHz signals (300 feet with obstacles, or 1500 feet with line of sight, which are limited in both range and power efficiency.

## **Our goal** 

This project stemmed from my curiosity about wireless communication technologies. As I began to explore the topic, I learned about the potential of the 433MHz band, which offers unique benefits in terms of both range and power efficiency. The idea to experiment with this frequency band came from a desire to better understand how communication systems work in real-world scenarios.

## **Why we want to do this**

This research has the potential to make a significant impact on various applications, particularly in scenarios that require long-range communication with minimal power consumption. For example, it could benefit search and rescue operations, enabling rescuers to transmit crucial information over longer distances with lower power requirements than traditional methods. This capability would be particularly valuable in remote areas where cell towers are absent, allowing rescue teams to communicate and share information across miles by using a relay network of devices. In such settings, a chain of devices could bounce the signal from one to the next, creating a much-needed lifeline for rescuers and people in distress.

## **Experiment No.1**

Working bench prototype. Our first goal was to get hardware assembled in a mostly working fashion, with software that works enough to test it. This consisted of WEEKS at a desk with wires and circuit boards all over the place.

## 

## **Experiment No.2**

Range testing. Now that we had working prototypes, how far could these go? We wanted to give it a fair LOS (line of sight) test, So I took it outside, and began to walk down the sidewalk, walking backwards so that there would be nothing in between the 2 devices. 10 feet later. It stopped working. Why was the range so short? Well, A very important piece of information was skimmed over in the datasheet, a transmit power of only 0.1mW (Most WiFi devices fall around 10mW \- 20mW). Ultimately, we consider this a failure as the devices could not go further than a WiFi connection.

## **Background information**

But why did we use 433MHz over the standard 2.4GHz? In theory, the longer wavelength offered at 433MHz would allow for better penetration through walls and other various objects. These long waves can also be beneficial when it comes to diffraction, they will bounce and bend around objects instead of being absorbed like the 2.4GHz. The 433MHz band is also far less noisy, less devices use it, leading to less interference.  
With all of these benefits why doesn’t more consumer grade electronics use this technology? It's used more than you'd expect, your car’s keyfob uses it, but in terms of bandwidth, it is slow. The transceivers we choose to use call out a maximum of 50 kbps. 

## **Refection**

If I had more time to work on this project, we likely would also include a 2-way amplifier to boost the signal strength, and a directional antenna. Both of these options would have significantly boosted the result. However performing either of these actions would have resulted in us needing to get our Amauter Ham Radio Licenses (side note, we safely complied with FCC part 15\. Regulations by not surpassing 1 mW of transmit power). I do have plans to revisit this project and see just how far I can push it once I do get licensed.  
