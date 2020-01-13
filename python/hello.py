import site
g1=12
g2=0
def p():
    g1=2
    print(g1)

class Car:
    st="0000sdfsd"
    def __init__(self, speed=0):
        self.speed = speed
        self.odometer = 0
        self.time = 0

    def do(self):
        print("--------------------")
        print(Car.st)
        print(self.st)
        print(self.uu2)
        u1=None
        print(u1)
        u1="1111asdfsadf"
        print(u1)
        g1=1
        g2 = 1
        print(g1, g2)
        def do1():
            g2=3
            print(1111, g2)
        def do2():
            g2=4
            print(2222, g2)
        do1=do2
        do1()

    def say_state(self):
        print("I'm going {} kph!".format(self.speed))

    def accelerate(self):
        self.speed += 5

    def brake(self):
        self.speed -= 5

    def step(self):
        self.odometer += self.speed
        self.time += 1

    def average_speed(self):
        if self.time != 0:
            return self.odometer / self.time
        else:
            pass



if __name__ == '__main__':

    class inputErr(BaseException):
        def __init__(self,str):
            pass

    def ask_ok(prompt, retries=4, reminder='Please try again!'):
        while True:
            ok = input(prompt)
            if ok in ['y', 'ye', 'yes']:
                return True
            if ok in ('n', 'no', 'nop', 'nope'):
                return False
            retries = retries - 1
            if retries < 0:
                raise ValueError('invalid user response')
            #raise inputErr('sdaf')
            print(reminder)

    #ask_ok("sadfasdf")
    #print(p1)
    print("---------------")
    p()
    my_car = Car()
    print(my_car.st)
    print("*******")
    my_car.st="asdfsdaf"
    print(my_car.st)
    print(Car.st)
    Car.st="0000"
    print(Car.st)
    print(my_car.st)
    Car.uu="xxxxxxxxxxxxxxxxx"
    my_car.uu2="tttttttttttttttttttttttt"
    print(my_car.uu2)
    print(Car.uu)
    my_car.do()
    print(site.getuserbase())
    while True:
        action = input("What should I do? [A]ccelerate, [B]rake, "
                 "show [O]dometer, or show average [S]peed?").upper()
        if action not in "ABOS" or len(action) != 1:
            print("I don't know how to do that")
            continue
        if action == 'A':
            my_car.accelerate()
        elif action == 'B':
            my_car.brake()
        elif action == 'O':
            print("The car has driven {} kilometers".format(my_car.odometer))
        elif action == 'S':
            print("The car's average speed was {} kph".format(my_car.average_speed()))
        my_car.step()
        my_car.say_state()