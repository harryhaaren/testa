Feature: Adding Numbers
  Scenario Outline: Simple Test
    Given a number <one>
    And another number  <two>
    When added together
    Then the result must be <answer>
    Examples:
      |    one    | two     | answer   |
      |      0    |    0    |     0    |
      |      5    |    7    |    12    |
      |     50    |   20    |    70    |
      | 0xefff    |  0x1    |  0xf000  |
      | 0xff0f    |  0x1    |  0xff10  |
      | 0xfffe    |  0x1    |  0xffff  |
      | 0xffff    |  0x1    |     0x0  |
